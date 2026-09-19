# Signal handler improvements

* Would be better if:
1. Each signal had an id
2. Each signal had a refreshment interval
3. Each signal had a value
4. And each signal had a validity value

This would allow for creating signals with different update intervals. It would also allow for checking the signal validity (if the signal has not been refreshed within the refreshment interval, then it is invalid and should not be used). The validity could be checked in some global check_and_read() function. Signal IDs would make it possible to only log
the ids and the signal values enabling smaller logs.