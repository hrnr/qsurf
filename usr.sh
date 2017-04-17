#!/bin/sh

# dmenu appearence
normbgcolor='#222222'
normfgcolor='#aaaaaa'
selbgcolor='#535d6c'
selfgcolor='#ffffff'

# files
barhistoryf=~/.surf/barhistory.txt
findhistoryf=~/.surf/findhistory.txt
browserhistoryf=~/.surf/history.txt
bookmarksf=~/.surf/bookmarks.txt

# how many history entries to preserve
history_count=100

# action to perform
action=$1
# current url in the browser
uri=$2

dmenu="dmenu -nb $normbgcolor -nf $normfgcolor \
       -sb $selbgcolor -sf $selfgcolor"

s_write_bookmarksf() { # file value
    tags="$($dmenu -p 'bookmark tags:' | tr ' ' ':')"
    [ -n "$2" ] && (grep -F -v "$2" "$1" > "$1.temp"; mv "$1.temp" "$1"; echo "t-:$tags $2" >> "$1")
}

s_write_historyf() { # file value preserve_count
    [ -n "$2" ] && (grep -F -v "$2" "$1" | tail -n $3 > "$1.temp"; mv "$1.temp" "$1"; echo "$2" >> "$1")
}

s_clean_historyf() { # file preserve_count
    uniq "$1" | tail -n $2 > "$1.temp"; mv "$1.temp" "$1";
}

urlencode() {
    perl -MURI::Escape -e "print uri_escape('$1');"
}

case $action in
"find")
    find="`tac $findhistoryf 2>/dev/null | $dmenu -p find:`"
    echo "$find"
    s_write_historyf $findhistoryf "$find" $history_count
    ;;
"bookmark")
    s_write_bookmarksf $bookmarksf "$uri"
    ;;
"navigate_raw")
    uri=`echo "$uri" | $dmenu -p "uri:"`
    echo "$uri"
    ;;
"navigate")
    s_clean_historyf "$browserhistoryf" 500
    sel=`(echo "$uri"; tac $barhistoryf 2> /dev/null; tac $bookmarksf 2> /dev/null; tac $browserhistoryf 2> /dev/null) |
         $dmenu -l 5 -p "uri [?gwyx]:"`
    [ -z "$sel" ] && exit
    # after ? space is not required
    if echo "$sel" | grep -q '^\?[^ ]'; then
        opt=$(echo "$sel" | cut -c 1)
        arg=$(echo "$sel" | cut -c 2-)
    else
        opt=$(echo "$sel" | cut -d ' ' -f 1)
        arg=$(echo "$sel" | cut -d ' ' -f 2-)
    fi
    save=0
    query=$(urlencode "$arg")
    case "$opt" in
    '?') # google for it
        uri="http://www.google.com/search?q=$query"
        save=1
        ;;
    "g") # google for it
        uri="http://www.google.com/search?q=$query"
        save=1
        ;;
    "w") # wikipedia
        uri="http://wikipedia.org/wiki/$query"
        save=1
        ;;
    "y") # youtube
        uri="http://www.youtube.com/results?search_query=$query&aq=f"
        save=1
        ;;
    "x") # delete
        for f in $barhistoryf $findhistoryf $bookmarksf $browserhistoryf; do
            grep -F -v "$arg" "$f" > "$f.temp"; mv "$f.temp" "$f"
        done
        exit;
        ;;
    "t-:"*|"h:"*) # bookmark or history -> strip tags
        uri="$arg"
        save=0
        ;;
    *)
        uri="$sel"
        save=1
        ;;
    esac

    # only set the uri; don't write to file
    [ $save -eq 0 ] && echo "$uri"
    # set the url and write exactly what the user inputed to the file
    [ $save -eq 1 ] && (echo "$uri"; s_write_historyf $barhistoryf "$sel" $history_count)
    # make sure to exit with zero (last comparison)
    exit 0
    ;;
*)
    echo Unknown xprop >&2
    ;;
esac