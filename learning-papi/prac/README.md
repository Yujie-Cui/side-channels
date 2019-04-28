# Notes on Using PAPI

## Terminology
**Native Events**: All the possible CPU events (specific to CPU)
**Preset Events**: Native events that PAPI maps to a common API across all CPUs (ex. "L1 cache hits" event name may be different across CPUs).

## Events
To see all preset events, run:
```
papi/src/utils/papi_avail
```
I saved my output to: `ALL_PRESET_EVENTS.out`
There is also a [Present Events Table](http://icl.cs.utk.edu/projects/papi/presets.html) online, but it may not be up-to-date.

## Issues
1. [User Guide](http://icl.cs.utk.edu/projects/papi/files/documentation/PAPI_USER_GUIDE.htm#C_AND_FORTRAN_CALLING_INTERFACES) calls the struct `PAPI_preset_info_t`, which does not exist... I changed it to `PAPI_event_info_t`, which compiles, but now it fails to get `PAPI_TOT_INS` event...
