# What's this
Scripts to reverse-engineer Google Pixel 6 bootloader.

# Description
Currently, just one WIP script which parses the symbol table and outputs
<addr, name> pairs, and a Ghidra script to rename symbols.

# Use
```
./parse.sh  2>/dev/null  > /tmp/gs101_addr_map
```

Run ghidra, click `Display Script Manager` (play), create a new script and
past contents of `GS101_RenameFunctionsFromSymtabParsed.java`
