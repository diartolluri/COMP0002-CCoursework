#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"
graphics_mode=1
text_only=0
commands_only=0
arguments=("$@")
for ((index = 0; index < ${#arguments[@]}; index++)); do
    case "${arguments[index]}" in
        --generate) index=$((index + 2)); continue ;;
        --preset|-f|--file|--save|--seed|--loops|--solver|--budget|--delay)
            index=$((index + 1)); continue ;;
        --headless) graphics_mode=0 ;;
        --graphics|--commands) graphics_mode=1 ;;
    esac
    case "${arguments[index]}" in
        --preview|--help) text_only=1 ;;
        --commands) commands_only=1 ;;
    esac
done

if [ "$graphics_mode" -eq 0 ] || [ "$text_only" -eq 1 ]; then
    make --no-print-directory all >&2
    exec ./build/maze "$@"
fi

make --no-print-directory draw >&2
if [ "$commands_only" -eq 1 ]; then
    exec ./build/maze-draw "$@"
fi

if [ ! -f drawapp-5.0.jar ]; then
    echo 'Place the course drawapp-5.0.jar in c-coursework first.' >&2
    exit 1
fi
if [ -n "${JAVA_HOME:-}" ]; then
    java_command="$JAVA_HOME/bin/java"
elif [ -x .runtime/java/Contents/Home/bin/java ]; then
    java_command=.runtime/java/Contents/Home/bin/java
else
    java_command=java
fi
if ! "$java_command" -version >/dev/null 2>&1; then
    echo 'Java 21 or newer is required. Set JAVA_HOME to its installation directory.' >&2
    exit 1
fi

./build/maze-draw "$@" | "$java_command" -jar drawapp-5.0.jar
