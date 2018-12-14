export URL=https://smartthings-sonoff.herokuapp.com/

rm -rf target
mkdir target
#create index.html
REGEX_URL=$(echo $URL| sed -e 's/\//\\\//g')
export index0=$(cat index.html \
  | sed -e "s/<URL>/${REGEX_URL}/g" \
  )
echo  "${index0}"> target/index0.html
echo -n -e "\00" >>target/index0.html
xxd -i target/index0.html >../SmartThingsSonoff/index.h
rm -rf target
