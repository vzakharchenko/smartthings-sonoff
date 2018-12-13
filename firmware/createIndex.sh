cat  sonoffLine.html >index.html
echo -n -e "\00" >>index.html
xxd -i index.html >../SmartThingsSonoff/index.h



