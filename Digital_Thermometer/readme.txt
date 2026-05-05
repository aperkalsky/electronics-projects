Save 000 - the first working version
        Highlights:
        -----------
        - output to RS-232
        - Led blinks during measurement
        Problems:
        ---------
        (1) wait loops need to be adjusted -> recalculate
        (2) russian strings are too long -> choose another words
        (3) temperatures 0.5 and -0.5 will be not displayed perfect -> check code
        (4) LED blinking is too short -> add an artificial delay after both measurements
        (5) LCD refresh is visible -> (a)hold a shadow copy of display and update the differences only
										or
                                      (b)refresh only the fields where temperature is displayed and leave
						   strings unchanged
        (6) write also in Russian appropriate string when sensor is missing
        (7) Felix advised to provide an option to modify the length of period
            between two measurements. This can be done by removing serial
            port from the application and using two serial port pins of uC as
            an input from the DIP switches, so four variants can be selected.

Save 001 - the first version release
        Highlights:
        -----------
        - still output to serial port
        - LED blinks during measurement
        - intro screen with version number (V1.0) added
        - revised display time for IDs
        - fixed problems 1,2,4,5,6. (Note: (4) was a hardware problem and
          (5) was fixed by removing ClearDisplay() call from LCD update function)
        Problems:
        ---------
        - check (3) and implement (7) in next release



