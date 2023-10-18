cmd_/home/pi/Desktop/ung_dung_may_tinh/dongco.mod := printf '%s\n'   dongco.o | awk '!x[$$0]++ { print("/home/pi/Desktop/ung_dung_may_tinh/"$$0) }' > /home/pi/Desktop/ung_dung_may_tinh/dongco.mod
