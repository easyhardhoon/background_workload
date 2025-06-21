export DISPLAY=:0
echo 3 > /proc/sys/vm/drop_caches
sudo swapoff /dev/zram0
sudo swapoff /dev/zram1
sudo swapoff /dev/zram2
sudo swapoff /dev/zram3
sudo swapon /dev/zram0
sudo swapon /dev/zram1
sudo swapon /dev/zram2
sudo swapon /dev/zram3
