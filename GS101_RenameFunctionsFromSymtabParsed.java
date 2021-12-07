/* ###
 * IP: GHIDRA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Given a routine, show all the calls to that routine and their parameters.
//    Place the cursor on a function (can be an external .dll function).
//    Execute the script.
//    The decompiler will be run on everything that calls the function at the cursor
//    All calls to the function will display with their parameters to the function.
//
//   This script assumes good flow, that switch stmts are good.
//
//@category Functions

import java.io.File;
import java.io.FileNotFoundException;
import java.math.BigInteger;
import java.util.*;

import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.framework.options.ToolOptions;
import ghidra.framework.plugintool.util.OptionsService;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressFactory;
import ghidra.program.model.address.AddressSpace;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.MemoryAccessException;
import ghidra.program.model.pcode.*;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;

public class GS101_RenameFunctionsFromSymtabParsed extends GhidraScript {

    private Address lastAddr = null;
    private Map<Long, String> addrMap = null;

    void loadAddrMap()
    {
        addrMap = new HashMap<>();
        //addrMap.put(0xffff0000f889a3e4L, "Dual_Project");
        try {
            File fin = new File("/tmp/gs101_addr_map");
            Scanner sc = new Scanner(fin);
            while (sc.hasNextLine()) {
                String num_str = sc.next();
                long addr = new BigInteger(num_str, 16).longValue();
                String sym_name = sc.next();
                addrMap.put(addr, sym_name);
            }
        }
        catch (Exception e) {
            println(e.toString());
        }
    }

    @Override
    public void run() throws Exception {
        loadAddrMap();

        Listing listing = currentProgram.getListing();

        for (Long addr: addrMap.keySet()) {
            Address addrFromMap = currentProgram.getMinAddress().getNewAddress(addr);
            String nameFromMap = addrMap.get(addr);
            Function func = listing.getFunctionContaining(addrFromMap);

            if (func == null) {
                println("No Function at address " + addrFromMap);
                currentProgram.getSymbolTable().createLabel(addrFromMap, nameFromMap, SourceType.USER_DEFINED);
                continue;
            }
            else {
                println("Found function at addr" + func + " " + addr);
                try {
                    func.setName(nameFromMap, SourceType.USER_DEFINED);
                }
                catch (Exception e) {
                    println("Failed to set function name to " + func + " " + e.toString());
                }
            }
        }
    }
}

