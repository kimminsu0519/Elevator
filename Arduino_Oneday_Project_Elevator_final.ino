const int FLOOR = 3;
const int BETWEEN_FLOORS = 2;
const int MAX_FLOOR = FLOOR + (FLOOR - 1) * BETWEEN_FLOORS;
const float FLOOR_DELAY_TIME = 1000;

const int RED_LED_PINS[FLOOR] = {2, 6, 10};
const int GREEN_LED_PINS[FLOOR] = {3, 7, 11};
const int YELLOW_LED_PINS[BETWEEN_FLOORS * (FLOOR - 1)] = {4, 5, 8, 9};
const int BUTTON_PIN_LIST[FLOOR] = {A0, A1, A2};
const int EMERGENCY_LED_PIN = 13;
const int EMERGENCY_BUTTON_PIN = A5;

enum ElevatorDirection
{
  DOWN = 0,
  UP
};

enum ElevatorMoveStatus
{
  STOP = 0,
  MOVING
};

enum CallStatus
{
  NOTCALL = 0,
  CALL
};

enum UseStatus
{
  CAN_USE = 0,
  CANT_USE
};

CallStatus floor_status_list[FLOOR] = {NOTCALL};
CallStatus previous_button_state_list[FLOOR] = {NOTCALL};
CallStatus current_button_state_list[FLOOR] = {NOTCALL};
unsigned long now_time;
unsigned long last_moving_time = 0;
int elevator_position = 1;
ElevatorMoveStatus move_status = STOP;
ElevatorDirection direction = UP;
UseStatus moving_call_onetime_check = CAN_USE;
UseStatus moving_sec_onetime_check[BETWEEN_FLOORS + 1] = {CAN_USE};

void checkFloorButton();
CallStatus isCall(CallStatus status_list[], int size);
ElevatorDirection getCallDirection(CallStatus status_list[], int size, int pos, ElevatorDirection dir);
void onLED();
void onElevator();


/////////////////////////////////////////////////////////////        main      ///////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);

  for (int i = 0; i < FLOOR; i++) {
    pinMode(RED_LED_PINS[i], OUTPUT);
    pinMode(GREEN_LED_PINS[i], OUTPUT);
    pinMode(BUTTON_PIN_LIST[i], INPUT);
  }
  for (int i = 0; i < BETWEEN_FLOORS * (FLOOR - 1); i++) {
    pinMode(YELLOW_LED_PINS[i], OUTPUT);
  }
  pinMode(EMERGENCY_LED_PIN, OUTPUT);
  pinMode(EMERGENCY_BUTTON_PIN,INPUT);

  digitalWrite(RED_LED_PINS[elevator_position - 1], HIGH);
}

void loop() 
{
  now_time = millis();

  Serial.println("=== LOOP 시작 ===");

  // Serial.print("now_time: ");
  // Serial.println(now_time);

  // Serial.print("last_moving_time: ");
  // Serial.println(last_moving_time);

  // Serial.print("elevator_position: ");
  // Serial.println(elevator_position);

  Serial.print("direction: ");
  Serial.println(direction == UP ? "UP" : "DOWN");

  Serial.print("move_status: ");
  Serial.println(move_status == MOVING ? "MOVING" : "STOP");

  Serial.print("moving_call_onetime_check: ");
  Serial.println(moving_call_onetime_check == CANT_USE ? "CANT_USE" : "CAN_USE");

  // Serial.print("moving_sec_onetime_check: ");
  // for (int i = 0; i < BETWEEN_FLOORS + 1; i++)
  // {
  //   Serial.print(moving_sec_onetime_check[i] == CAN_USE ? "CAN " : "CANT ");
  // }
  // Serial.println();

  // Serial.print("floor_status_list: ");
  // for (int i = 0; i < FLOOR; i++)
  // {
  //   Serial.print(floor_status_list[i]);
  //   Serial.print(" ");
  // }
  // Serial.println();

  Serial.println("=================");



  checkFloorButton();

  onLED();

  if(move_status == STOP)
  {
    if(isCall(floor_status_list, FLOOR) == CALL)
    {
      if(moving_call_onetime_check == CAN_USE)
      {
        direction = getCallDirection(floor_status_list, FLOOR, elevator_position, direction);
      }
      last_moving_time = now_time;
      move_status = MOVING;
      delay(100);
    }
    else
    {
      return;
    }
  }
  else
  {
    onElevator();

    if(isCall(floor_status_list, FLOOR) == CALL)
    {
      if(moving_call_onetime_check == CAN_USE)
      {
        moving_call_onetime_check = CANT_USE;
        direction = getCallDirection(floor_status_list, FLOOR, elevator_position, direction);
        return;
      }
      else
      {
        return;
      }
    }
    else
    {
      return;
    }
  }
}
/////////////////////////////////////////////////////////////        main      ///////////////////////////////////////////////

////////////////////////////////////////////////////////////        isCall      ///////////////////////////////////////////////
CallStatus isCall(CallStatus status_list[], int size)
{
  for (int i = 0; i < size; i++)
  {
    if (status_list[i] == CALL)
    {
      return CALL;
    }
  }
  return NOTCALL;
}
////////////////////////////////////////////////////////////        isCall      ///////////////////////////////////////////////

///////////////////////////////////////////////////////        getCallDirection      ///////////////////////////////////////////////
ElevatorDirection getCallDirection(CallStatus status_list[], int size, int now_pos, ElevatorDirection dir)
{
  // Serial.println("getCallDirection() doing");
  int call_pos = now_pos; // 기본값지정
  // 호출된 층 찾기
  for(int i = 0; i < size; i++)
  {
    if(status_list[i] == CALL)
    {
      call_pos = (i * 3) + 1;
      break;
    }
  }

  if(call_pos == now_pos)
  {
    return dir;
  }

  if(call_pos < elevator_position)
  {
    return DOWN;
  }
  else
  {
    return UP;
  }
}
///////////////////////////////////////////////////////        getCallDirection      ///////////////////////////////////////////////

////////////////////////////////////////////////////////////        onLED      ///////////////////////////////////////////////
void onLED()
{
  if(elevator_position % 3 == 1)
  {
    digitalWrite(RED_LED_PINS[elevator_position / 3], HIGH);
    digitalWrite(GREEN_LED_PINS[elevator_position / 3], LOW);
    floor_status_list[elevator_position / 3] = NOTCALL;
  }
  else
  {
    digitalWrite(YELLOW_LED_PINS[(elevator_position - 2) - ((elevator_position - 2) / 3)], HIGH);
  }


  for(int i = 0; i < FLOOR; i++)
  {
    if(floor_status_list[i] == CALL)
    {
      digitalWrite(GREEN_LED_PINS[i], HIGH);
    }
    else
    {
      digitalWrite(GREEN_LED_PINS[i], LOW);
    }
  }

  return;
}
////////////////////////////////////////////////////////////        onLED      ///////////////////////////////////////////////

//////////////////////////////////////////////////////////        onElevator      ///////////////////////////////////////////////
void onElevator()
{
  // 0~1초 동안
  if(now_time - last_moving_time < FLOOR_DELAY_TIME)
  {
    // Serial.println("0~1 doing");
    if(moving_sec_onetime_check[0] == CAN_USE)
    {
      digitalWrite(RED_LED_PINS[elevator_position / 3], LOW);

      if(direction == UP)
      {
        elevator_position++;
      }
      else
      {
        elevator_position--;
      }
      moving_sec_onetime_check[0] = CANT_USE;
      return;
    }
    else
    {
      return;
    }
  }
  // 1~2초 동안
  else if(now_time - last_moving_time < FLOOR_DELAY_TIME * 2)
  {
    // Serial.println("1~2 doing");
    if(moving_sec_onetime_check[1] == CAN_USE)
    {
      digitalWrite(YELLOW_LED_PINS[(elevator_position - 2) - ((elevator_position - 2) / 3)], LOW);

      if(direction == UP)
      {
        elevator_position++;
      }
      else
      {
        elevator_position--;
      }
      moving_sec_onetime_check[1] = CANT_USE;
      return;
    }
    else
    {
      return;
    }
  }  
  // 2~3초 동안
  else if(now_time - last_moving_time < FLOOR_DELAY_TIME * 3)
  {
    // Serial.println("2~3 doing");
    if(moving_sec_onetime_check[2] == CAN_USE)
    {
      digitalWrite(YELLOW_LED_PINS[(elevator_position - 2) - ((elevator_position - 2) / 3)], LOW);

      if(direction == UP)
      {
        elevator_position++;
      }
      else
      {
        elevator_position--;
      }
      moving_sec_onetime_check[2] = CANT_USE;
      return;
    }
    else
    {
      return;
    }
  }
  // 3초 이후
  else
  {
    // Serial.println("after doing");
    move_status = STOP;
    moving_call_onetime_check = CAN_USE;
    for(int i = 0; i < BETWEEN_FLOORS + 1; i++)
    {
      moving_sec_onetime_check[i] = CAN_USE;
    }
    return;
  }
}
//////////////////////////////////////////////////////////        onElevator      ///////////////////////////////////////////////

///////////////////////////////////////////////////////        checkFloorButton      ///////////////////////////////////////////////
void checkFloorButton()
{
  for (int i = 0; i < FLOOR; i++)
  {
    current_button_state_list[i] = (CallStatus)digitalRead(BUTTON_PIN_LIST[i]);

    if ((current_button_state_list[i] == CALL) && (previous_button_state_list[i] == NOTCALL))
    {
      if (floor_status_list[i] == CALL) // 이미 켜져있다면, OFF하기
      {
        floor_status_list[i] = NOTCALL;
      }
      else
      {
        floor_status_list[i] = CALL;
      }
    }
  }
  // 버튼 이전상태 저장
  for (int i = 0; i < FLOOR; i++)
  {
    previous_button_state_list[i] = current_button_state_list[i];
  }
}
///////////////////////////////////////////////////////        checkFloorButton      ///////////////////////////////////////////////