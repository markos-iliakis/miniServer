lines=$( wc -l < "$2" )
if [ -d "$1" ] && [ -f "$2" ] && [[ $3 =~ ^[0-9]+$ ]] && [[ $4 =~ ^[0-9]+$ ]] && [[ "$lines" -ge 10000 ]]; then

    if [ ! -z "$1" ]; then
        echo "Warning: directory not empty, purging..."
        rm -r $1si*
    fi

    # make the directories along with the files
    declare -a arr=()
    for (( i = 0; i < $3; i++ )); do
        mkdir "$1/site_$i"
        for (( j = 0; j < $4; j++ )); do
            ra="_$RANDOM.html"
            arr+=("/site_$i/page$i$ra")
            # touch "$1/site_$i/page$i$r"
        done
    done

    # make f (the number of links to make that point to the same web site)
    let "f=$4/2+1"

    # make q (the number of links to make that point to other directories)
    let "q=$3/2+1"

    # initialize the visited array
    declare -a visited=()
    declare -a temp_visited=()
    for (( i = 0; i < $3*$4; i++ )); do
        visited[$i]=0
    done

    # write the texts in files
    for (( i = 0; i < $3; i++ )); do
        echo "Creating web site $i"
        for (( j = 0; j < $4; j++ )); do

            let "index=$i*$4+$j"
            countf=0
            countq=0

            # make random k
            low=1
            let "high=$lines-2000"
            # echo "$high"
            k=$((low + RANDOM%(1+high-low)))

            # make random m
            low=1000
            high=2000
            m=$((low + RANDOM%(1+high-low)))

            # make l_copy (lines to copy)
            let "l_copy=$m/($f+$q)"

            printf "\tCreating page ${arr[$index]} with $l_copy lines starting at line $k\n"
            printf "<!DOCTYPE html>\n<html>\n\t<body>\n" >> "$1${arr[$index]}"

            for (( z = 0; z < $3*$4; z++ )); do
                temp_visited[$z]=0
            done

            # copy the lines needed
            for (( r = k; r < lines; r+=l_copy )); do

                # for (( z = 0; z < $3*$4; z++ )); do
                #     printf "visited : ${temp_visited[$z]}\n"
                # done

                # make random x (index for links)
                if [ "$countf" -lt "$f" ]; then
                    #link inside website
                    let "low=$i*$4"
                    let "high=$low+$4-1"
                    let "low_bound=$high+1"
                    let "high_bound=$high+1"
                    # echo "Inner link"
                    ((countf++))
                elif [ "$countq" -lt "$q" ]; then
                    #link to another website
                    low=0
                    let "high=$3*$4-1"
                    let "low_bound=$i*$4"
                    let "high_bound=$low_bound+$4-1"
                    # echo "Outer link $r $k $l_copy"
                    ((countq++))
                else
                    break
                fi

                x=$((low + RANDOM%(1+high-low)))
                # echo "$x"
                while true; do

                    if ((x>=low_bound && x<=high_bound)); then
                        # echo "case1"
                        x=$((low + RANDOM%(1+high-low)))
                    elif [ "${temp_visited[$x]}" -eq 1 ]; then
                        # echo "case2"
                        x=$((low + RANDOM%(1+high-low)))
                    elif ((x==index)); then
                        # echo "case3"
                        x=$((low + RANDOM%(1+high-low)))
                    else
                        break
                    fi
                    # echo "$x"
                    # sleep 1
                done

                # echo "$x"

                cat "$2" | tail -$r | head -$l_copy >> "$1${arr[$index]}"

                # set the links
                printf "\t\tAdding link to ${arr[$x]}\n"
                printf "<a href=""${arr[$x]}"">${arr[$x]}</a>\n" >> "$1${arr[$index]}"
                visited[$x]=1
                temp_visited[$x]=1
            done

            printf "\n</body>\n</html>\n" >> "$1${arr[$index]}"
        done
    done

    # echo "$f , $q"

    # check visited array
    not_visited=0
    for (( i = 0; i < $3*$4; i++ )); do
        if [ "$visited" -eq 0 ]; then
            not_visited=1
        fi
    done

    if [ "$not_visited" -eq 1 ]; then
        echo "Not all pages have at least one incoming link"
    else
        echo "All page have at least one incoming link"
    fi

else
    echo "Wrong arguments given"
fi
