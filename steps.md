# Initial config

### **Tux 2**
E0 -> ETH7

### **Tux 3**
E0 -> ETH2

### **Tux 4**
S0 -> Console (para init inicial) <br>
E0 -> ETH4 <br>
E1 -> ETH5

### **Router**
Porta da net de cima -> ETH1 (router) <br>
ETH2 -> ETH9 (mtik)

```
(Tux 2)
    (console)
        systemctl restart networking
        ifconfig 172.16.Y1.1/24

(Tux 3)
    (console)
        systemctl restart networking
        ifconfig 172.16.Y0.1/24

(Tux 4)
    (console)
        systemctl restart networking
        ifconfig 172.16.Y1.253/24
        ifconfig 172.16.Y0.254/24

    (Switch)
        system reset-configuration
        interface bridge add name=bridgeY0
        interface bridge add name=bridgeY1

        interface bridge port remove [find interface=ether2]
        interface bridge port remove [find interface=ether4]
        interface bridge port remove [find interface=ether5]
        interface bridge port remove [find interface=ether7]

        interface bridge port add interface=ether2 bridge=bridgeY0
        interface bridge port add interface=ether4 bridge=bridgeY0
        interface bridge port add interface=ether5 bridge=bridgeY1
        interface bridge port add interface=ether7 bridge=bridgeY1


(Tux 2, 3, 4)
    (console)
        echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

(Tux 4)
    (console)
        echo 1 > /proc/sys/net/ipv4/ip-forward

(Tux 3)
    (console)
        route add default gw 172.16.Y0.254

(Tux 2)
    (console)
        route add default gw 172.16.Y1.253
```

## Tirar cabo da consola para o MTIK (linha dos GNU)

```
(Tux 4)
    (Router)
        system reset-configuration
        ip address add address=172.16.S.Y9/24 interface ether1 (internet)
        ip address add address=172.16.Y1.254/24 interface ether2

    (Switch)
        interface bridge port remove [find interface=ether9]
        interface bridge port add interface=ether9 bridge=bridgeY1

        ip address add address=172.16.Y1.254/24 interface=ether2 (AINDA NAO SEI ESTE)

(Tux 2)
    (console)
        echo 0 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
        echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects
        ip route add 172.16.Y0.0/24 via 172.16.Y1.253

(Tux 4)
    (Router)
        ip route add dst-address=0.0.0.0/0 gateway=172.16.S.254
        ip route add dst-address=172.16.Y0.0/24 gateway=172.16.Y1.253
```
    

## Experiencia 5

```
(Tux 2,3,4)
    (console)
        sudo nano /etc/resolv.conf
        nameserver 172.16.S.1
``` 

## Experiencia 6

```
(Tux 3)
    (console)
        make
        ./ftpdownloader ftp://ftp.up.pt/pub/gnu/gnu.ps.gz
``` 

