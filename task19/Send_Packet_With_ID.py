from scapy.all import *

# Define the target IP
target_ip = "192.168.52.129" # my VM IP

# Define your custom data
custom_data = "fake_id_123"

# Craft the ICMP packet with custom data in the payload
packet = IP(dst=target_ip)/ICMP()/custom_data

# Send the packet
send(packet)