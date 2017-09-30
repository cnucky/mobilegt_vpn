
#example:
#    ./startCapture.sh tun0 4cd4ccc

if [ ! $# -eq 2 ];then
    echo "example:"
    echo "./startCapture.sh tun0 4cd4ccc"
    exit 0
fi

DATA_HOME=/home/ubuntu/vpnserver/pcap_data
IF_NAME=$1
DEVICE_ID=$2
DATA_DIR=$DATA_HOME/$DEVICE_ID
#DURA_SEC=60
DURA_SEC=3600

#sudo chown root:root /home/lab/vpnserver/pcap_data/
#sudo tshark -i tun0 -b duration:60 -w /home/lab/vpnserver/pcap_data/1.pcap

if [ ! -d $DATA_DIR ];then
    echo $DATA_DIR not exist!
    #cat password | sudo -S mkdir -p $DATA_DIR
    sudo -S mkdir -p $DATA_DIR
fi
#echo sudo tshark -i $IF_NAME -b duration:$DURA_SEC -F pcap -w $DATA_DIR/1.pcap
echo sudo dumpcap -i $IF_NAME -b duration:$DURA_SEC -P -w $DATA_DIR/1.pcap
#cat password | sudo -S tshark -i $IF_NAME -b duration:$DURA_SEC -w $DATA_DIR/1.pcapng
#sudo tshark -i $IF_NAME -b duration:$DURA_SEC -F pcap -w $DATA_DIR/1.pcap
sudo dumpcap -i $IF_NAME -b duration:$DURA_SEC -P -w $DATA_DIR/1.pcap
