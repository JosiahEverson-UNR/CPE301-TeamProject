# CPE 301
### Team Project Assignment

#### Goal
- Create an evaporation cooling system.
    - An evaporation cooler pulls air in from the outside through a pad that is soaked with water.
    - The evaporation of the water cools and humidifies the air.
#### Restrictions
- Use Git to manage source code.
#### Submission
- Project Overview Document
    - An overview of the design and any constraints on the system *(operating temperatures, power requirements, etc.)*.
    - Pictures of the final system.
    - A link to a video of the system in operation.
    - A complete schematic.
    - Links to all relevant specification sheets from the components used.
    - A link to the GitHub repository.
    - A test plan based on the system requirements.
- GitHub Repository
    - All source code.
    - Reasonable and helpful commit comments.
    - Evidence of contributions from all team members.
- Video of the System
    - Show system in operation.
    - Some narration explaining the operation.
    - Possible subdued background music.
#### Project Requirements
- Monitor the water levels in a reservoir.
    - Print an alert when the water level is too low.
- Monitor and display the current air temperature and humidity on an LCD screen.
- Start and stop a fan motor as needed when the temperature falls out of a specified range *(high or low)*.
- Allow a user to use a control to adjust the angle of an output vent from the system.
- Allow a user to enable or disable the system using an on/off button.
- Record the time and date every time the motor is turned on or off.
    - This information should be transmitted to a host computer *(over USB)*.
#### Evaporation Cooler States
- **All States**
    - Humidity and temperature should be continuously monitored and reported on the LCD screen.
    - System should respond to changes in vent position.
    - Stop button should turn off motor *(if on)* and system should go to disabled state.
- **Disabled State** *(yellow light)*
    - No monitoring of temperature or water should be performed.
- **Idle State** *(green light)*
    - System should monitor temperature and transition to running state when temperature is more than the threshold *(you determine the threshold)*.
        - Exact time stamp *(using real time clock)* should record transition times.
    - Water level should be continuously monitored and state changed to error if level is too low.
- **Error State** *(ONLY red light)*
    - Motor should be off and not start regardless of temperature.
    - System should transition to **Idle State** as soon as water is at acceptable level.
    - Error message should be displayed on the LCD.
- **Running State** *(ONLY blue light)*
    - Motor should be on.
    - System should transition to **Idle State** as soon as temperature drops below lower threshold.
    - System should transition to **Error State** if water becomes too low.
