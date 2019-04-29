# Notes on Using PAPI

## Terminology
* **Native Events**: All the possible CPU events (specific to CPU)
* **Preset Events**: Native events that PAPI maps to a common API across all CPUs (ex. "L1 cache hits" event name may be different across CPUs).

## Events
To see all preset events, run:
```
papi/src/utils/papi_avail
```
I saved my output to: `ALL_PRESET_EVENTS.out`
There is also a [Present Events Table](http://icl.cs.utk.edu/projects/papi/presets.html) online, but it may not be up-to-date.

## Issues
1. [User Guide](http://icl.cs.utk.edu/projects/papi/files/documentation/PAPI_USER_GUIDE.htm#C_AND_FORTRAN_CALLING_INTERFACES) calls the struct `PAPI_preset_info_t`, which does not exist... I changed it to `PAPI_event_info_t`, which compiles, ~~but now it fails to get `PAPI_TOT_INS` event...~~ and works (see Issue 2).

2. Events can't be read if `perf_event` is disabled (`paranoid=3`) by Linux. To enable, run:
```
sudo sh -c 'echo -1 >/proc/sys/kernel/perf_event_paranoid'
```
[Source](https://stackoverflow.com/questions/32308175/papi-avail-no-events-available)

3. I have no idea what `NATIVE_MASK` is in the examples in the User Guide...

4. This compiler warning: `gcc: warning: /usr/local/lib/libpapi.a: linker input file unused because linking not done`

5. Undefined references...
```
hAPI.c:(.text+0x24): undefined reference to `PAPI_num_counters'
hAPI.c:(.text+0x74): undefined reference to `PAPI_start_counters'
hAPI.c:(.text+0xac): undefined reference to `PAPI_read_counters'
hAPI.c:(.text+0xfc): undefined reference to `PAPI_accum_counters'
hAPI.c:(.text+0x136): undefined reference to `PAPI_stop_counters'
```
