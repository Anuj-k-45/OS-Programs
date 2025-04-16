get_user_info() {
    local username=$1
    if grep -q "^$username:" /etc/passwd; then
        user_info=$(grep "^$username:" /etc/passwd)
        echo "Here’s the info for $username: $user_info"
    else
        echo "Sorry, couldn’t find $username."
    fi
}

compare_files() {
    local file1=$1
    local file2=$2

    if [[ ! -f "$file1" ]]; then
        echo "Oops! File '$file1' doesn’t exist."
        return 1
    fi
    if [[ ! -f "$file2" ]]; then
        echo "Oops! File '$file2' doesn’t exist."
        return 1
    fi

    sorted_file1=$(mktemp)
    sorted_file2=$(mktemp)
    sort "$file1" | uniq > "$sorted_file1"
    sort "$file2" | uniq > "$sorted_file2"

    diff_output=$(diff "$sorted_file1" "$sorted_file2")
    if [[ -z "$diff_output" ]]; then
        echo "The files are the same."

        read -p "Do you want to remove the second file ($file2)? (y/n): " delete_choice
        if [[ "$delete_choice" == "y" ]]; then
            rm "$file2"
            echo "I’ve removed '$file2'."
        fi
    else
        echo "The files are different."
        echo "$diff_output"
    fi

    rm "$sorted_file1" "$sorted_file2"
}

echo "Please enter the usernames you want to check (separate them with spaces):"
read -a usernames

found_users=()
not_found_users=()

for username in "${usernames[@]}"; do
    if grep -q "^$username:" /etc/passwd; then
        found_users+=("$username")
    else
        not_found_users+=("$username")
    fi
done

echo "Here’s the user summary:"
echo "Found users: ${found_users[@]}"
echo "Not found users: ${not_found_users[@]}"

while true; do
    echo "Let’s compare some files. Please provide the first file:"
    read file1

    echo "Now provide the second file:"
    read file2

    compare_files "$file1" "$file2"

    read -p "Would you like to compare another pair of files? (y/n): " compare_another
    if [[ "$compare_another" != "y" ]]; then
        break
    fi
done

echo "All done! Thanks for using the script."
