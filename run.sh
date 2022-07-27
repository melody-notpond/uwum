Xephyr -br -ac -noreset -sw-cursor -screen 800x600 :1 &> /dev/null &
sleep 1
DISPLAY=:1 ./uwum
