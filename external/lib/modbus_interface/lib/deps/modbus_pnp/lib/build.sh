
[[ ! -d build ]] && mkdir "build"

touch build/modbus_pnp_single_header.h
cat include/mb_definitions.h > build/modbus_pnp_single_header.h
cat include/mb_structs.h >> build/modbus_pnp_single_header.h
cat include/modbus_pnp.h >> build/modbus_pnp_single_header.h
cat src/modbus_pnp.c >> build/modbus_pnp_single_header.h

sed -i 's/#include.*mb_definitions.h.*/\/\/&/g'  build/modbus_pnp_single_header.h
sed -i 's/#include.*mb_structs.h.*/\/\/&/g'  build/modbus_pnp_single_header.h
sed -i 's/#include.*modbus_pnp.h.*/\/\/&/g'  build/modbus_pnp_single_header.h

gcc -c -fPIC -o build/modbus_pnp.o src/modbus_pnp.c

ar -rcs build/libmodbus_pnp_static.a build/modbus_pnp.o
