DT=$(date +"%m_%d_%Y")
HR=$(date +"%H")
mkdir -p log
mkdir -p log/$DT
(bin/./updateCTasks >> log/$DT/$HR &) & 