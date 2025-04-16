if [ ! -f routine.txt ]; then
    echo "Looks like the routine file is missing, so I'll create one for you..."
    cat > routine.txt <<EOL
Monday: Math (Theory) 9:00 AM - 10:30 AM, English (Practical) 11:00 AM - 1:00 PM
Tuesday: Physics (Theory) 9:00 AM - 10:30 AM, Chemistry (Practical) 11:00 AM - 1:00 PM
Wednesday: Computer Science (Theory) 9:00 AM - 10:30 AM, Math (Practical) 11:00 AM - 1:00 PM
Thursday: Biology (Theory) 9:00 AM - 10:30 AM, English (Practical) 11:00 AM - 1:00 PM
Friday: Physics (Theory) 9:00 AM - 10:30 AM, Chemistry (Practical) 11:00 AM - 1:00 PM
Saturday: Computer Science (Practical) 9:00 AM - 11:00 AM, Biology (Practical) 11:30 AM - 1:00 PM
EOL
    echo "I've set up a default routine for you."
fi

hour=$(date +"%H")

if [ $hour -lt 12 ]; then
    echo "Good Morning!"
elif [ $hour -lt 18 ]; then
    echo "Good Afternoon!"
else
    echo "Good Evening!"
fi

while true; do
    echo
    echo "What would you like to do? (print, day, count, average, modify, exit):"
    read command

    case $command in
        print)
            echo "Here's your full routine:"
            cat routine.txt
            ;;
        
        day)
            echo "Which day would you like to see the routine for? (e.g., Monday, Tuesday):"
            read day
            grep -i "^$day" routine.txt || echo "Sorry, no routine found for $day."
            ;;
        
        count)
            theoretical=$(grep -o "(Theory)" routine.txt | wc -l)
            practical=$(grep -o "(Practical)" routine.txt | wc -l)
            echo "You've got $theoretical theoretical classes and $practical practical classes."
            ;;
        
        average)
            total_classes=$(grep -o "[A-Za-z]* (Theory|Practical)" routine.txt | wc -l)
            total_days=$(grep -o "^[A-Za-z]*" routine.txt | wc -l)
            average=$(echo "$total_classes / $total_days" | bc)
            echo "On average, you have $average classes per day."
            ;;
        
        modify)
            echo "Which day's routine would you like to update? (e.g., Monday, Tuesday):"
            read day
            echo "What’s the new routine for $day? (e.g., Math (Theory) 9:00 AM - 10:30 AM, English (Practical) 11:00 AM - 1:00 PM):"
            read new_routine
            sed -i "/^$day/c\\$day: $new_routine" routine.txt
            echo "$day's routine has been updated."
            ;;
        
        exit)
            echo "Thanks for using the routine manager. Have a fantastic day!"
            break
            ;;
        
        *)
            echo "Hmm, that’s not a valid option. Could you try again?"
            ;;
    esac
done
