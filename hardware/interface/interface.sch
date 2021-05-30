EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L interface-rescue:GND #PWR014
U 1 1 58D2EBAA
P 6650 4600
F 0 "#PWR014" H 6650 4350 50  0001 C CNN
F 1 "GND" H 6650 4450 50  0000 C CNN
F 2 "" H 6650 4600 50  0001 C CNN
F 3 "" H 6650 4600 50  0001 C CNN
	1    6650 4600
	1    0    0    -1  
$EndComp
Connection ~ 7050 4000
$Comp
L Connector_Generic:Conn_01x02 J8
U 1 1 58D2EE7D
P 9150 4400
F 0 "J8" H 9150 4550 50  0000 C CNN
F 1 "CONN_01X02" V 9250 4400 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B02B-XH-A_1x02_P2.50mm_Vertical" H 9150 4400 50  0001 C CNN
F 3 "" H 9150 4400 50  0001 C CNN
	1    9150 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 4500 8800 4500
Text Notes 9350 4500 0    60   ~ 0
BUTTON 4
NoConn ~ 9850 2250
$Comp
L Device:R R1
U 1 1 598071E6
P 2750 3200
F 0 "R1" V 2700 3050 50  0000 C CNN
F 1 "1K" V 2750 3200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P5.08mm_Vertical" V 2680 3200 50  0001 C CNN
F 3 "" H 2750 3200 50  0001 C CNN
	1    2750 3200
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 5980722B
P 2750 3400
F 0 "R3" V 2700 3250 50  0000 C CNN
F 1 "5K1" V 2750 3400 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P5.08mm_Vertical" V 2680 3400 50  0001 C CNN
F 3 "" H 2750 3400 50  0001 C CNN
	1    2750 3400
	0    1    1    0   
$EndComp
$Comp
L interface-rescue:GND #PWR02
U 1 1 598072C9
P 2950 3900
F 0 "#PWR02" H 2950 3650 50  0001 C CNN
F 1 "GND" H 2950 3750 50  0000 C CNN
F 2 "" H 2950 3900 50  0001 C CNN
F 3 "" H 2950 3900 50  0001 C CNN
	1    2950 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 4000 7050 4000
$Comp
L interface-rescue:C C2
U 1 1 59807C8B
P 6650 4350
F 0 "C2" H 6675 4450 50  0000 L CNN
F 1 "100u" H 6675 4250 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P5.00mm" H 6688 4200 50  0001 C CNN
F 3 "" H 6650 4350 50  0001 C CNN
	1    6650 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 4000 6650 4200
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5CDFF210
P 2300 3400
F 0 "J1" H 2218 2975 50  0000 C CNN
F 1 "Buttons 1-3" H 2218 3066 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B04B-XH-A_1x04_P2.50mm_Vertical" H 2300 3400 50  0001 C CNN
F 3 "~" H 2300 3400 50  0001 C CNN
	1    2300 3400
	-1   0    0    1   
$EndComp
Wire Wire Line
	2500 3500 2950 3500
Wire Wire Line
	2950 3500 2950 3900
$Comp
L Device:R R2
U 1 1 5CE009C8
P 2750 3300
F 0 "R2" V 2700 3100 50  0000 L CNN
F 1 "2K2" V 2750 3300 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P5.08mm_Vertical" V 2680 3300 50  0001 C CNN
F 3 "~" H 2750 3300 50  0001 C CNN
	1    2750 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	4100 3100 4050 3100
Wire Wire Line
	4050 3100 4050 5000
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 5CE0A21E
P 4300 3100
F 0 "J2" H 4250 2650 50  0000 L CNN
F 1 "Arduino" H 4250 2550 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 4300 3100 50  0001 C CNN
F 3 "~" H 4300 3100 50  0001 C CNN
	1    4300 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 3000 4050 3000
Wire Wire Line
	4050 3000 4050 2850
$Comp
L interface-rescue:MountingHole-Mechanical H1
U 1 1 5CE975F5
P 1650 5300
F 0 "H1" H 1750 5346 50  0000 L CNN
F 1 "MountingHole" H 1750 5255 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 1650 5300 50  0001 C CNN
F 3 "~" H 1650 5300 50  0001 C CNN
	1    1650 5300
	1    0    0    -1  
$EndComp
$Comp
L interface-rescue:MountingHole-Mechanical H2
U 1 1 5CE979F8
P 1650 5700
F 0 "H2" H 1750 5746 50  0000 L CNN
F 1 "MountingHole" H 1750 5655 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 1650 5700 50  0001 C CNN
F 3 "~" H 1650 5700 50  0001 C CNN
	1    1650 5700
	1    0    0    -1  
$EndComp
$Comp
L interface-rescue:MountingHole-Mechanical H3
U 1 1 5CE97D65
P 1650 6050
F 0 "H3" H 1750 6096 50  0000 L CNN
F 1 "MountingHole" H 1750 6005 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 1650 6050 50  0001 C CNN
F 3 "~" H 1650 6050 50  0001 C CNN
	1    1650 6050
	1    0    0    -1  
$EndComp
$Comp
L interface-rescue:MountingHole-Mechanical H4
U 1 1 5CE9804D
P 1650 6400
F 0 "H4" H 1750 6446 50  0000 L CNN
F 1 "MountingHole" H 1750 6355 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 1650 6400 50  0001 C CNN
F 3 "~" H 1650 6400 50  0001 C CNN
	1    1650 6400
	1    0    0    -1  
$EndComp
Text Notes 4400 3350 0    60   ~ 0
GND
Text Notes 4400 3050 0    60   ~ 0
+5V
Text Notes 4400 3150 0    60   ~ 0
BUTTON (D12)
Text Notes 4400 3250 0    60   ~ 0
SW1 (A5)
Wire Wire Line
	4100 3300 3700 3300
Wire Wire Line
	3700 3300 3700 3500
Wire Wire Line
	3700 3500 2950 3500
Connection ~ 2950 3500
Wire Wire Line
	2500 3200 2600 3200
Wire Wire Line
	2600 3300 2500 3300
Wire Wire Line
	2500 3400 2600 3400
Wire Wire Line
	2950 2200 7050 2200
Wire Wire Line
	2950 2200 2950 2750
Wire Wire Line
	2950 3050 2950 3200
Wire Wire Line
	2950 3400 2900 3400
Wire Wire Line
	2900 3300 2950 3300
Connection ~ 2950 3300
Wire Wire Line
	2950 3300 2950 3400
Wire Wire Line
	2900 3200 2950 3200
Connection ~ 2950 3200
Wire Wire Line
	2950 3200 2950 3300
Wire Wire Line
	2950 3200 4100 3200
$Comp
L Device:R R4
U 1 1 5CEB36BC
P 2950 2900
F 0 "R4" H 3020 2946 50  0000 L CNN
F 1 "1K" H 3020 2855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P5.08mm_Vertical" V 2880 2900 50  0001 C CNN
F 3 "~" H 2950 2900 50  0001 C CNN
	1    2950 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 2200 7050 2850
$Comp
L Device:R R?
U 1 1 60566F03
P 5075 2850
F 0 "R?" V 4868 2850 50  0000 C CNN
F 1 "20" V 4959 2850 50  0000 C CNN
F 2 "" V 5005 2850 50  0001 C CNN
F 3 "~" H 5075 2850 50  0001 C CNN
	1    5075 2850
	0    1    1    0   
$EndComp
Wire Wire Line
	4925 2850 4050 2850
Wire Wire Line
	5225 2850 7050 2850
Connection ~ 7050 2850
Wire Wire Line
	7050 2850 7050 4000
$Comp
L Device:R R5
U 1 1 6060E0D3
P 7050 4200
F 0 "R5" H 7120 4246 50  0000 L CNN
F 1 "1K" H 7120 4155 50  0000 L CNN
F 2 "" V 6980 4200 50  0001 C CNN
F 3 "~" H 7050 4200 50  0001 C CNN
	1    7050 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 4400 7050 4350
Wire Wire Line
	7050 4400 8950 4400
Wire Wire Line
	7050 4050 7050 4000
$Comp
L interface-rescue:GND #PWR?
U 1 1 6061156C
P 8800 4875
F 0 "#PWR?" H 8800 4625 50  0001 C CNN
F 1 "GND" H 8805 4702 50  0000 C CNN
F 2 "" H 8800 4875 50  0001 C CNN
F 3 "" H 8800 4875 50  0001 C CNN
	1    8800 4875
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 4500 8800 4875
Wire Wire Line
	7050 4400 7050 5000
Wire Wire Line
	4050 5000 7050 5000
Connection ~ 7050 4400
Wire Wire Line
	6650 4500 6650 4600
$EndSCHEMATC
