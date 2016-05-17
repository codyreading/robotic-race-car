/*Stores constants for the speeds of the motors, TURN = Turn Speed, ANG = Turn Angle,
HS = Half Speed, FS = Full Speed, SS = Super Speed, SCHECK1 = # of colour sensor registers before registering that its out of a turn
SCHECK 2 = # of colour sensor registers before registering that its read a lap */
int const TURN = 10,TS = -20, FS = -40, SS = -70, SCHECK1 = 2, SCHECK2 = 10;

//Waits for a button press, and returns the pressed button
int buttonSelect ()
{
   while (nNxtButtonPressed == -1){}
   int b = nNxtButtonPressed;
   while (nNxtButtonPressed != -1){}
   return b;
}

//Displays menu and waits for user input of number of laps, returns number of laps selected
int menu()
{
 int button[2];
 do
 {
    nxtDisplayString(1, "Select number of");
    nxtDisplayString(2, "laps:");
    nxtDisplayString(4, "2 Laps (Right)");
    nxtDisplayString(5, "3 Laps (Left)");
    nxtDisplayString(6, "4 Laps (Middle)");

    button[0] = buttonSelect() + 1;
    eraseDisplay();

    nxtDisplayString(0, "You have");
    nxtDisplayString(1, "selected %d laps.", button[0]);
    nxtDisplayString(3, "Is this correct?");
    nxtDisplayString(4, "Yes(Middle)");
    nxtDisplayString(5, "No (Left)");

    button[1] = buttonSelect();
    eraseDisplay();
 } while (button[1] == 2);

 eraseDisplay();
 nxtDisplayString(0, "On your marks...");
 wait10Msec(100);
 nxtDisplayString(1, "Get set...");
 wait10Msec(100);
 nxtDisplayString(2, "Go!!!");

 return button[0];
}

//Activates the sweeper arm
void sweep ()
{
	motor[motorC] = 50;
	while (nMotorEncoder[motorC] < 135){}
	motor[motorC] = -50;
	while(nMotorEncoder[motorC] > 25){}
	motor[motorC] = 0;
}

//Converts and displays time from 10's of milliseconds into minutes and seconds
void timeDisplay (int line, const string text, int time)
{
  float sec = time /100.0;
  if (sec > 0)
    nxtDisplayString (line, "%s: %d:%.2f", text, time/6000, sec);
  else
    nxtDisplayString (line, "%s: %d:0%.2f", text, time/6000, sec);
}

//Incremenents the lapCheck variable depending on if it needs to register that its reading the white line or coming off the white line
int lapUpdate(int lapCheck, bool startLine)
{
	if ((SensorValue[S1] == 6 || SensorValue[S4] == 6) && startLine || SensorValue[S1] != 6 && SensorValue[S4] != 6 && !startLine)
  	lapCheck ++;
  else
  	lapCheck = 0;

  return lapCheck;
}

//Incremenents the turnCheck variable depending on if it needs to register that it needs to stop the left turn or the right turn
int turnUpdate(int turnCheck, int dir)
{
	if (dir == -1 && SensorValue[S4] != 4 || dir == 1 && SensorValue[S1] != 4)
		 turnCheck ++;
	else
		 turnCheck = 0;

	return turnCheck;
}

void crash()
{
  eraseDisplay();
  motor[motorA]=0;
  motor[motorB]=0;
  nxtDisplayString(0,"You have crashed");
  nxtDisplayString(1,"Please place car ");
  nxtDisplayString(2,"back on track");
  nxtDisplayString(3,"and push button");
  nxtDisplayString(4,"to continue race.");

	while(nNxtButtonPressed == -1){}
	while(nNxtButtonPressed != -1){}

	//Resets steering back to straight
	if (nMotorEncoder[motorB] < 0)
	{
		motor[motorB]=TURN;
		while (nMotorEncoder[motorB] < 0) {}
	}
	else if (nMotorEncoder[motorB] > 0)
	{
		motor[motorB]=-TURN;
		while (nMotorEncoder[motorB] > 0) {}
	}
  eraseDisplay();
	motor[motorA] = FS;
}

//Speeds up the robot for ahort time, annd then slows it down
void speedBoost()
{
  motor[motorA] = SS;
  wait10Msec(50);
  motor[motorA] = FS;
}

//Written by: Cody Reading
//Controls steering of the robot
int steer(int lapCheck)
{

  motor[motorA] = TS;
  time10[T2] = 0;
  int turnCheck = 0;
  int dir = 1; //Stores which way it is turning, -1 is right, 1 is left

  //Detects lane on left side, will turn right
  if (SensorValue[S4] == 4)
  {
	  motor[motorB]=TURN;
		while (nMotorEncoder[motorB] < 20) {}
		dir = -1;
  }

  //Detects lane of right side, will turn left
  else if (SensorValue[S1] == 4)
  {
	  motor[motorB]=-TURN;
	  while (nMotorEncoder[motorB] > -35) {}
  }

	motor[motorB]=0;

	//Colour Sensor often flickers values, this statement checks that it has truly changed colours so it can exit the turn
	while (turnCheck < SCHECK1)
	{
	  turnCheck = turnUpdate(turnCheck, dir);
		lapCheck = lapUpdate(lapCheck, 1);

		//Will check for lap complete, crash, or speed boost while in a turn
		if (lapCheck >= SCHECK2)
    	return lapCheck;
		if (SensorValue[S1] == 4 && SensorValue[S4] == 4)
			 crash();
		if (SensorValue[S1] == 2 || SensorValue[S4] == 2)
			speedBoost();
	}
	motor[motorB] = dir * TURN;
	while (dir * nMotorEncoder[motorB] < 0){}

	motor[motorB] = 0;
	motor[motorA] = FS;
  return lapCheck;
}

task main()
{
  int lap = menu();
  int time[4];
  int totTime = 0, lapCheck = 0;

  SensorType[S1] = sensorCOLORFULL; // Right Colour
  SensorType[S2] = sensorTouch;
  SensorType[S3] = sensorSONAR;
  SensorType[S4] = sensorCOLORFULL; //Left Colour
  nMotorEncoder[motorB] = 0;
  nMotorEncoder[motorC] = 0;
  motor[motorA] = FS;

  for (int i = 0; i < lap; i++)
  {
  	time10[T1] = 0;
    lapCheck = 0;
  	nxtDisplayString(7, "Lap %d", i+1);
  	do
  	{
  		 //Robot will perform different tasks depending on colour passed
  	   if (SensorValue[S1] == 2 || SensorValue[S4] == 2) //Blue
         speedBoost();
  	   else if (SensorValue[S1] == 4 || SensorValue[S4] == 4)
  	     lapCheck = steer(lapCheck); /*Will steer the robot if it detects yellow,
  	                                  but if it detects white during a turn while return that it return that as well */

       if (SensorValue[S3] <= 30 || SensorValue[S2] == 1) //Detects object to sweep out of the way
         sweep();

       //Will avoid another update if the lapCheck variable has already incremeneted enough times due to the steer function
       if (lapCheck < SCHECK2)
        lapCheck = lapUpdate(lapCheck,1);

    }while (lapCheck < SCHECK2);

    eraseDisplay();
    time[i] = time10[T1]; //stores lap time
    totTime += time[i];
    lapCheck = 0;

    //Waits for the robot to come off the white line in order to start a new lap
    while (lapCheck < SCHECK2)
    	lapCheck = lapUpdate(lapCheck,0);
  }

  motor[motorA] = 0;
  wait10Msec(300);
  timeDisplay(0, "Total", totTime);
  timeDisplay(2, "Lap 1", time[0]);
  timeDisplay(3, "Lap 2", time[1]);

	if (lap > 2)
  	timeDisplay(4, "Lap 3", time[2]);
  if (lap > 3)
  	timeDisplay(5, "Lap 4", time[3]);
  wait10Msec(1000);
}
