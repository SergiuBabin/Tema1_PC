# Tema1_PC
Router

Tema Protocoale de Comunicatie Nr. 1

Babin Sergiu
Grupa: 324CC

Tema a fost realizata timp de 4-5 zile in limbajul C.

Problemele intilnite au fost mai mult in lucrul cu limbajul folosit,
decat in rezolvarea temei.
De asemenea am avut o problema care mi-a crash-it de 3 ori masina virtuala,
din cauza spatiului de stocare, la un moment dac cand porneam checkerul
imi spunea ca nu mai am spatiu suficient pentru a rula, am incercat sa 
caut fisierul care imi ocupa cea mai multa memorie si ca sa vezi, era 
fisierul cu tema :D (10 GB)(Black Magic :D), (nu stiu ce sa intimplat dar
am descarcat din nou fisierul si dupa a mers).

Inplementare Tema:

-> Parsare tabela de rutare: am facut o functie (read_rtable()) in care citesc
string cu string din rtable.txt si adaug intrun vector de tip "route_table_entry"
pentru a transforma din string in "uint32_t" am folosit functia "inet_pton(...)".


-> Implementati protocolul ARP : FAIL :(
Am folosit tabela statica ARP din fisierul arp_table.txt


-> Procesul de dirijare: am initializat cate un obiect pentru fiecare protocol 
utilizat (IPv4, Ethernet, ICMP), apoi am tratat  fiecare caz corespunzator, daca
este un pachet IP destinat routerului, daca este un pachet cu TTL <= 1 sau un 
pachet catre o adresa inexistenta in tabela de rutare, daca este un pachet cu
checksum gresit.
Daca trece de toate conditiile impuse anterior atunci trimite packetul cu functia
send_packet(...);

->Implementare suport pentru protocolul ICMP: pentru fiecare caz in parte,
am modificat corespunzator Packetul, cu functia setIphdr_Icmphdr(), dupa care am
trimis pachetul corespunzator.


VmCheckerul - Imi ofera punctaj diferit faca de checkerul local, am pus si un screenshot 
cu punctajul de pe local.



