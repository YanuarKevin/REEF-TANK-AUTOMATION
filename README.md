# REEF-TANK-AUTOMATION
Bahan-bahan:
1. WEMOS D1 MINI
2. 74HC595
3. Module Relay 4ch
4. DS18B20
5. DHT11
6. LCD I2C
7. BOX
8. DC Female Conector
9. Servo 180 degree
10. Buzzer
11. FAN 12v 8" x 2
12. PSU 5V 2A min
13. Capasitor
14. PCB
15. Jumper
16. Baut & Mur
17. Plat induktor salinity meter (pengembangan selanjutnya)

Fungsi / cara kerja:
1. Mengukur suhu & kelembapan ruang (DHT11)
2. Mengukur suhu air aquarium (DS18B20)
3. Suhu ruang sebagai pembanding & perkiraan suhu air yang di capai
4. LCD menampilkan nama Development saat booting
5. LCD menampilkan suhu ruang & suhu air (menit 1-5)
6. LCD tiap menit 5 menampilkan Suhu air + Target suhu min-max (menit 5:30)
7. LCD tiap menit 5:30 menampilkan suhu & kelembaban ruang (menit 6)
8. LCD tiap menit 6 menampilkan mode auto fan & date (menit 6:30)
9. LCD tiap menit 6:30 menampilkan salinitas + Target salinitas min-max (menit 7) (pengembangan selanjutnya)
10. LCD clear + teks ketika feeding (servo berputar) 
11. Relay A1 = FAN
12. Relay A2 = FAN PSU
13. Relay A3 & A4 = Kosong
14. Buzzer menyala 3 detik 6 ketukan ketika suhu air max 1x mengikuti notif hp
15. Buzzer menyala 4 detik 8 ketukan ketika salinitas drop & up mengikuti notif hp (pengembangan selanjutnya)
16. Notif 1x suhu ruang + air max
17. Notif 1x salinitas drop / up (pengembangan selanjutnya)

Aplikasi:
1. Gauge Suhu Air (V10)
2. Gauge Suhu Ruang (V11)
3. Gauge Kelembaban (V12)
4. Gauge Salinitas (V13) (pengembangan selanjutnya)
5. Button V0 Feeding
6. Button V1 FAN mode auto
7. Button V3 FAN manual  
8. Button V4 Relay A3
9. Button V5 Relay A4
10. Timer Feeding (V0)
11. Timer Fan Manual (V2)
12. Timer Relay (A3)
13. Timer Relay (A4)
14. Input V6 min suhu air
15. Input V7 max suhu air
16. Input V8 min salinitas (pengembangan selanjutnya)
17. Input V9 max salinitas (pengembangan selanjutnya)
18. Chart Suhu Air (V10)
19. Chart Suhu ruang (V11)
20. Chart Suhu kelembaban (V12)
21. Chart Suhu Air & ruang (V10 & V11)
22. Chart Salinitas (V13)
23. Notif Email
24. Notif Aps
