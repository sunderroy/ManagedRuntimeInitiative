/*
 * Copyright 1998-1999 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
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
 */

/* @test
 * @bug 4127826
 *
 * @summary synopsis: need to download factories for use with custom socket
 * types
 * @author Ann Wollrath
 *
 * @library ../../../../testlibrary
 * @build TestLibrary RMID JavaVM StreamPipe
 * @build Echo
 * @build EchoImpl
 * @build EchoImpl_Stub
 * @build UseCustomSocketFactory
 * @run main/othervm/policy=security.policy/timeout=120 UseCustomSocketFactory
 */

import java.io.*;
import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;

public class UseCustomSocketFactory {

    public static void main(String[] args) {

        String[] protocol = new String[] { "", "compress", "xor" };

        System.out.println("\nRegression test for bug 4127826\n");

        TestLibrary.suggestSecurityManager("java.rmi.RMISecurityManager");

        try {
            LocateRegistry.createRegistry(TestLibrary.REGISTRY_PORT);
        } catch (Exception e) {
            TestLibrary.bomb("creating registry", e);
        }

        for (int i = 0; i < protocol.length; i++) {

            System.err.println("test policy: " +
                               TestParams.defaultPolicy);

            JavaVM serverVM = new JavaVM("EchoImpl",
                                         "-Djava.security.policy=" +
                                         TestParams.defaultPolicy,
                                         protocol[i]);
            System.err.println("\nusing protocol: " +
                               (protocol[i] == "" ? "none" : protocol[i]));

            try {
                /* spawn VM for EchoServer */
                serverVM.start();

                /* lookup server */
                int tries = 8;
                Echo obj = null;
                do {
                    try {
                        obj = (Echo) Naming.lookup("//:" + TestLibrary.REGISTRY_PORT +
                                                   "/EchoServer");
                        break;
                    } catch (NotBoundException e) {
                        try {
                            Thread.sleep(2000);
                        } catch (Exception ignore) {
                        }
                        continue;
                    }
                } while (--tries > 0);

                if (obj == null)
                    TestLibrary.bomb("server not bound in 8 tries", null);

                /* invoke remote method and print result*/
                System.err.println("Bound to " + obj);
                byte[] data = ("Greetings, citizen " +
                               System.getProperty("user.name") + "!"). getBytes();
                byte[] result = obj.echoNot(data);
                for (int j = 0; j < result.length; j++)
                    result[j] = (byte) ~result[j];
                System.err.println("Result: " + new String(result));

            } catch (Exception e) {
                TestLibrary.bomb("test failed", e);

            } finally {
                serverVM.destroy();
                try {
                    Naming.unbind("//:" + TestLibrary.REGISTRY_PORT +
                                  "/EchoServer");
                } catch (Exception e) {
                    TestLibrary.bomb("unbinding EchoServer", e);

                }
            }
        }
    }
}
