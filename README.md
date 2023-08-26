# Hybrid-Network-Simulation
Steps to run program
1. Install ns3
2. Install required libraries like gnuplot("sudo apt install gnuplot")
3. Copy wired.cc and wireless.cc in ns3.37/scratch folder
4. Open terminal in ns3.37 folder
5. Enter given commands

Commands to run program :

1)   ./ns3 run scratch/wired.cc
2)   gnuplot Wired-Fairness.plt
3)   gnuplot Wired-Throughput.plt

This will run the simulation for the first network(wired) and will generate graphs for throughput and fairness
Some of the network flow statistics can be seen in terminal output also

Commands to run program :

1)   ./ns3 run scratch/wireless.cc
2)   gnuplot Wireless-Fairness.plt
3)   gnuplot Wireless-Throughput.plt

This will run the simulation for the first network(wired) and will generate graphs for throughput and fairness
Some of the network flow statistics can be seen in terminal output also
