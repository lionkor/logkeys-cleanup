# logkeys-cleanup

A simple utility to clean up logkeys log files.

Example:

Input
```
2024-01-21 14:56:19+0100 > <LCtrl>l<LCtrl>rtail <End>
2024-01-21 14:56:22+0100 > <Enter><LMeta><#+52><LMeta>asdasdasdwasd
2024-01-21 14:56:40+0100 > <Enter>asdhdfkadshfads
2024-01-21 14:56:41+0100 > <Enter>fa
```

Run command
```sh
# build this program
make main
# run it
cat /path/to/logfile | ./main

# alternatively
./main /path/to/logfile
```

Output
```
2024-01-21 14:56:19+0100 > lrtail 
2024-01-21 14:56:22+0100 > asdasdasdwasd
2024-01-21 14:56:40+0100 > asdhdfkadshfads
2024-01-21 14:56:41+0100 > fa
```

To save the output to a file, redirect stdout: `./main /path/to/logfile >output.log`.
