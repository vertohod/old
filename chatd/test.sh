#!/bin/bash

#for (( counter=1; counter<=1000; counter++ ))
#do
#	path="./cookie$counter.txt"
#	curl http://localhost:9292/view.json -c $path -b $path 1>/dev/null 2>/dev/null
#	curl http://localhost:9292/login.json -c $path -b $path --data "room=1&nickname=User_test_`date +%N`" 1>/dev/null 2>/dev/null
#	curl http://localhost:9292/create.json -c $path -b $path --data "room=1&name_room=Paradise&private=1" 1>/dev/null 2>/dev/null
#	curl http://localhost:9292/comein.json -c $path -b $path --data "room=1" 1>/dev/null 2>/dev/null
#	curl http://localhost:9292/send.json -c $path -b $path --data "room=1&message=Hello World!" 1>/dev/null 2>/dev/null
#done

#while [ 1 ]; do
#	for (( counter=1; counter<=1000; counter++ ))
#	do
#		path="./cookie$counter.txt"
#		curl http://localhost:9292/send.json -c $path -b $path --data "room=1&message=Some message with url: http://localhost:9292 `date +%N`" 1>/dev/null 2>/dev/null
#		sleep 1
#	done
#done

curl http://localhost:9292/view.json -c ./cookie1.txt -b ./cookie1.txt
curl http://localhost:9292/login.json -c ./cookie1.txt -b ./cookie1.txt --data "nickname=User_test_`date +%N`"
clear
curl http://localhost:9292/send.json -c ./cookie1.txt -b ./cookie1.txt --data "room=0&message=какой-то текст (москва.рф) в круглых скобках и [россия.рф] в квадратных"
#curl http://localhost:9292/send.json -c ./cookie1.txt -b ./cookie1.txt --data "room=0&message=How%20are%20you?&private=1&to=0"
curl http://localhost:9292/logout.json -c ./cookie1.txt -b ./cookie1.txt 1>/dev/null 2>/dev/null

#curl http://localhost:9292/view.json -c ./cookie2.txt -b ./cookie2.txt
#curl http://localhost:9292/login.json -c ./cookie2.txt -b ./cookie2.txt --data "room=0&nickname=User_test_2"

#curl -D- http://localhost:9292/comein.json -c ./cookie2.txt -b ./cookie2.txt --data "room=1"


# даем доступ второму пользователю
#curl http://localhost:9292/add.json -c ./cookie1.txt -b ./cookie1.txt --data "object=rallow&room=1&oids="

# Расшарить сессию
#curl http://localhost:9292/share.json -c ./cookie1.txt -b ./cookie1.txt --data "password=qwer"

# Создать алиас
#curl -D- http://localhost:9292/add.json -c ./cookie2.txt -b ./cookie2.txt --data "object=alias&nickname=Sergey&password=qwer"

#curl -I https://rupor.chat/view.json?mask=31 -c ./cookie.txt -b ./cookie.txt
