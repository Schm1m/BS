# BS Lab 1: Shellskripte by Tim and Tom

case $1 in
    # simple switch-case to determine which sub-programm to start

    -h | --help) # display help page
    # echo -e [string] return the following string with escaped characters enabled
    echo -e "Usage: \"bash (programm) -[option]\" \n \nOptions: \n "
    echo -e "\t[-h | --help] \n \t\t display this help page \n "
    echo -e "\t[-e | --email] \n \t\t return the amount of emails sent by each email adress \n "
    echo -e "\t[-c | --company] \n \t\t return the amount of emails sent over each domain"
    ;;

    -e | --email | "") # count number of emails sent by each adress
    # awk [/regex/] [file] (originally a script language to edit and analyze text row by row) returns every row containing the given regex (a common email regex)
    # cut [-d] [-f] cuts the input string at the given delimiter " " and returns the second field
    # sort sorts the input lines. if not present, uniq wont count up all lines correctly
    # sort [-n] sorts the given lines by a number
    awk '/From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}/' gcc_2023.08.txt | cut -d" " -f2 | sort | uniq -c | sort -n
    ;;

    -c | --company) # count number of emails sent over each domain
    # awk [/regex/] [file] (originally a script language to edit and analyze text row by row) returns every row containing the given regex (a common email regex)
    # cut [-d] [-f] cuts the input string at the given delimiter " " and returns the second field
    # cut [-d] [-f] cuts the input string at the given delimiter "@" and returns the second field
    # sort sorts the input lines. if not present, uniq wont count up all lines correctly
    # sort [-n] sorts the given lines by a number
    awk '/From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}/' gcc_2023.08.txt | cut -d" " -f2 | cut -d"@" -f2 | sort | uniq -c | sort -n
    ;;

    *) # return unknown parameter if given
    echo "unknown parameter \"$1\""
    ;;
esac

# instead of the command "awk '/From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}/' gcc_2023.08.txt" 
# we could have used "grep -E -o 'From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}' gcc_2023.08.txt" 
# to get the same result, if the usage of "grep" is mandatory for this Lab