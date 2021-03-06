/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */

/*
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LEFontInstance.h"
#include "OpenTypeTables.h"
#include "AnchorTables.h"
#include "MarkArrays.h"
#include "GlyphPositioningTables.h"
#include "AttachmentPosnSubtables.h"
#include "MarkToBasePosnSubtables.h"
#include "GlyphIterator.h"
#include "LESwaps.h"

LEGlyphID MarkToBasePositioningSubtable::findBaseGlyph(GlyphIterator *glyphIterator) const
{
    if (glyphIterator->prev()) {
        return glyphIterator->getCurrGlyphID();
    }

    return 0xFFFF;
}

le_int32 MarkToBasePositioningSubtable::process(GlyphIterator *glyphIterator, const LEFontInstance *fontInstance) const
{
    LEGlyphID markGlyph = glyphIterator->getCurrGlyphID();
    le_int32 markCoverage = getGlyphCoverage((LEGlyphID) markGlyph);

    if (markCoverage < 0) {
        // markGlyph isn't a covered mark glyph
        return 0;
    }

    LEPoint markAnchor;
    const MarkArray *markArray = (const MarkArray *) ((char *) this + SWAPW(markArrayOffset));
    le_int32 markClass = markArray->getMarkClass(markGlyph, markCoverage, fontInstance, markAnchor);
    le_uint16 mcCount = SWAPW(classCount);

    if (markClass < 0 || markClass >= mcCount) {
        // markGlyph isn't in the mark array or its
        // mark class is too big. The table is mal-formed!
        return 0;
    }

    // FIXME: We probably don't want to find a base glyph before a previous ligature...
    GlyphIterator baseIterator(*glyphIterator, (le_uint16) (lfIgnoreMarks /*| lfIgnoreLigatures*/));
    LEGlyphID baseGlyph = findBaseGlyph(&baseIterator);
    le_int32 baseCoverage = getBaseCoverage((LEGlyphID) baseGlyph);
    const BaseArray *baseArray = (const BaseArray *) ((char *) this + SWAPW(baseArrayOffset));
    le_uint16 baseCount = SWAPW(baseArray->baseRecordCount);

    if (baseCoverage < 0 || baseCoverage >= baseCount) {
        // The base glyph isn't covered, or the coverage
        // index is too big. The latter means that the
        // table is mal-formed...
        return 0;
    }

    const BaseRecord *baseRecord = &baseArray->baseRecordArray[baseCoverage * mcCount];
    Offset anchorTableOffset = SWAPW(baseRecord->baseAnchorTableOffsetArray[markClass]);
    const AnchorTable *anchorTable = (const AnchorTable *) ((char *) baseArray + anchorTableOffset);
    LEPoint baseAnchor, markAdvance, pixels;

    if (anchorTableOffset == 0) {
        // this means the table is mal-formed...
        glyphIterator->setCurrGlyphBaseOffset(baseIterator.getCurrStreamPosition());
        return 0;
    }

    anchorTable->getAnchor(baseGlyph, fontInstance, baseAnchor);

    fontInstance->getGlyphAdvance(markGlyph, pixels);
    fontInstance->pixelsToUnits(pixels, markAdvance);

    float anchorDiffX = baseAnchor.fX - markAnchor.fX;
    float anchorDiffY = baseAnchor.fY - markAnchor.fY;

    glyphIterator->setCurrGlyphBaseOffset(baseIterator.getCurrStreamPosition());

    if (glyphIterator->isRightToLeft()) {
        // dlf flip advance to local coordinate system
        glyphIterator->setCurrGlyphPositionAdjustment(anchorDiffX, anchorDiffY, -markAdvance.fX, -markAdvance.fY);
    } else {
        LEPoint baseAdvance;

        fontInstance->getGlyphAdvance(baseGlyph, pixels);
        fontInstance->pixelsToUnits(pixels, baseAdvance);

        // flip advances to local coordinate system
        glyphIterator->setCurrGlyphPositionAdjustment(anchorDiffX - baseAdvance.fX, anchorDiffY - baseAdvance.fY, -markAdvance.fX, -markAdvance.fY);
    }

    return 1;
}
