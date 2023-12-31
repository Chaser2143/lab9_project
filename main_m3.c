#include <stdio.h>

#define MISSILE_COMMAND_PART3

#include "config.h"
#include "gameControl.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "touchscreen.h"
#include "sound.h"
#include "display.h"

#define RUNTIME_S 240
#define RUNTIME_TICKS ((int)(RUNTIME_S / CONFIG_GAME_TIMER_PERIOD))

#define START_HEIGHT 80 //Start text height
#define START_WIDTH 40 //Start text width
#define START_WIDTH2 30 //Start text width
#define SECOND_WIDTH 160 //Second Width for stats
#define TEXT_SIZE 5 //Text size

volatile bool interrupt_flag;

uint32_t isr_triggered_count;
uint32_t isr_handled_count;

// Interrupt handler for game - use flag method so that it can be interrupted by
// the touchscreen tick while running.
void game_isr() {
  intervalTimer_ackInterrupt(INTERVAL_TIMER_0);
  interrupt_flag = true;
  isr_triggered_count++;
}

// Interrupt handler for touchscreen - tick directly
void touchscreen_isr() {
  intervalTimer_ackInterrupt(INTERVAL_TIMER_1);
  touchscreen_tick();
}

void game_win_cutscene(){
  display_fillScreen(CONFIG_BACKGROUND_COLOR);
  display_setCursor(START_WIDTH, START_HEIGHT);
  display_setTextColor(DISPLAY_WHITE);
  display_setTextWrap(true);
  display_setTextSize(TEXT_SIZE);
  display_print("YOU WON!");
}

void game_loss_cutscene(){
  display_fillScreen(CONFIG_BACKGROUND_COLOR);
  display_setCursor(START_WIDTH2, START_HEIGHT);
  display_setTextColor(DISPLAY_WHITE);
  display_setTextWrap(true);
  display_setTextSize(TEXT_SIZE);
  display_print("GAME OVER");
  sound_missionFailed();
}

// Milestone 3 test application
int main() {
  interrupt_flag = false;
  isr_triggered_count = 0;
  isr_handled_count = 0;

  display_init();
  touchscreen_init(CONFIG_TOUCHSCREEN_TIMER_PERIOD);
  gameControl_init();

  // Initialize timer interrupts
  interrupts_init();
  interrupts_register(INTERVAL_TIMER_0_INTERRUPT_IRQ, game_isr);
  interrupts_register(INTERVAL_TIMER_1_INTERRUPT_IRQ, touchscreen_isr);
  interrupts_irq_enable(INTERVAL_TIMER_0_INTERRUPT_IRQ);
  interrupts_irq_enable(INTERVAL_TIMER_1_INTERRUPT_IRQ);

  intervalTimer_initCountDown(INTERVAL_TIMER_0, CONFIG_GAME_TIMER_PERIOD);
  intervalTimer_initCountDown(INTERVAL_TIMER_1,
                              CONFIG_TOUCHSCREEN_TIMER_PERIOD);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_0);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_1);
  intervalTimer_start(INTERVAL_TIMER_0);
  intervalTimer_start(INTERVAL_TIMER_1);

  sound_initialize(); // Initializes the sound functionality
  //sound_runTest(); // Plays all the possible sounds to test them at the beginning of the game
  sound_introSong(); // Plays the intro song for the game start

  // Main game loop
  while ((isr_triggered_count < RUNTIME_TICKS) && !getGameStatus()) {
    while (!interrupt_flag)
      ;
    interrupt_flag = false;
    isr_handled_count++;

    gameControl_tick();
  }
  if (didYouWin()) {
    game_win_cutscene();
  }
  else {
    game_loss_cutscene();
  }
}