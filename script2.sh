# Store the process ID of the monitor program in a variable
monitor_pid=$!

# Define an array of tracer programs to test
tracer_programs=("tracer" "tracer" "tracer")


# Loop through the tracer programs and start each one
for tracer_program in "${tracer_programs[@]}"
do
    bin/./"$tracer_program" execute -u "sleep 5" &
done
for tracer_program in "${tracer_programs[@]}"
do
    bin/./"$tracer_program" execute -u "ls -l" &
done
for tracer_program in "${tracer_programs[@]}"
do
    bin/./"$tracer_program" execute -p "ls -l | grep | wc -l | sleep 3" &
done

    

# Wait for all tracer programs to finish
wait


# Stop the monitor program
kill $monitor_pid
