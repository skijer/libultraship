# Strips the all-'=' COFF symbols __create_def emits from switch tables; the linker
# reads them as alias entries and fails with LNK2001. Usage: cmake -D DEF_FILE=<def> -P
if(NOT EXISTS "${DEF_FILE}")
    return()
endif()

file(STRINGS "${DEF_FILE}" lines)
set(filtered "")
foreach(line IN LISTS lines)
    string(STRIP "${line}" stripped)
    if(NOT stripped MATCHES "^=+$")
        list(APPEND filtered "${line}")
    endif()
endforeach()
string(JOIN "\n" content ${filtered})
file(WRITE "${DEF_FILE}" "${content}\n")
