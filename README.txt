This file should contain:

-	Your names & UNI : Shriya Mahakala (srm2245), Cynthia Zhang (cz2975), Peter Yu (hy2846)
-	Homework assignment number: 5
-	Description for each part

The description should indicate whether your solution for the part is working
or not. You may also want to include anything else you would like to
communicate to the grader, such as extra functionality you implemented or how
you tried to fix your non-working code.

Part 1:

On 1 core, taskset1.txt took an average of 2170.93 ms and taskset2.txt took an average of 689.45 ms.
On 4 cores, taskset1.txt took an average of 100.05 ms and taskset2.txt took an average of 38.93 ms.

Part 5:

Type: average (ms), tail completion(ms)
Freezer_task1_4cores: 271.41, 1570
Freezer_task2_4cores: 123.73,  526
Heater_task1_4cores: 2320.21, 7276 
Heater_task2_4cores: 856.22, 5283 
fifo _task1_4cores: 1470.66, 3474
Fifo_task2_4cores: 485.1, 1809
Cfs_task1_4cores: 1652.61, 3490
Cfs_task2_4cores: 1329, 1329.18


Type: average(ms), tail completion(ms)
Freezer_task1_1cores: 848.84, 3293
Freezer_task2_1cores: 330.88, 1481
Heater_task1_1cores:  1164.49, 3907
Heater_task2_1cores:  331.65, 2189
fifo _task1_1cores: 2435.8, 3523
Fifo_task2_1cores: 510.34, 1821
Cfs_task1_1cores: 1959.45, 3600
Cfs_task2_1cores: 1350.18, 1804

Our freezer scheduler performed the best on 4 cores while the freezer and heater both performed well in 1 core for average time to completion. This might be at first surprising given that this implies that we beat cfs and fifo but the TA’s suggested that we pin all processes to a core for fifo with taskset so it was at a handicap for 4 cores. An interesting observation is that 1 core heater is faster than 4 core heater, which is probably due to lock contention. Freezer is also probably faster than fifo because it ensures that outliers that take a long time don't slow everyone else down by making them wait (in addition to pinning all tasks to 1 core thing that mihir told us).

The picture is largely the same for tail completions too. For tail completion time, the pattern is similar but even clearer. Freezer again performed best overall, with the lowest tail times across both workloads on both 1 core and 4 cores. This suggests that Freezer not only improved average performance, but also reduced long outlier delays. Heater had the worst tail completion times, especially on 4 cores (7276 ms and 5283 ms), showing that it was the least consistent scheduler and produced the largest slowdowns for some jobs. FIFO was generally in the middle: better than Heater, but worse than Freezer, and sometimes close to CFS. Overall, for both average and tail completion time, Freezer was the strongest scheduler, while Heater was the weakest, especially in the multicore case.

Description for each Part:

We implemented all parts and followed the spec/ed stem posts for our implementations. We confirmed that our kernel boots and runs freezer as the default scheduler. The testing we did for this was checking "ps" output, running run_tasks.sh with heavy task loads, watching youtube videos on our GUI, and confirming with print statements in our freezer code.
