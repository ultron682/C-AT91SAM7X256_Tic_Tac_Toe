#include <targets\AT91SAM7.h>

#include "pcf8833u8_lcd.h"

void timeDelay(int ms) {
  volatile int aa, bb;
  for (aa = 0; aa <= ms; aa++) {
    for (bb = 0; bb <= 3000; bb++) {
      __asm__("NOP");
    }
  }
}

char getChar()
{
  char znak;
  while ((US0_CSR & US0_CSR_RXRDY) == 0) {}
  znak = US0_RHR;
  return znak;
}

void sendString(const char * string)
{
  while ( * string) {
    while ((US0_CSR & US0_CSR_TXEMPTY) == 0) {}
    US0_THR = * string;
    string++;
  }
}

void drawCross(int row, int col) {
  int cellSize = 130 / 3;
  int centerX = (2 * col + 1) * cellSize / 2;
  int centerY = (2 * row + 1) * cellSize / 2;
  int crossSize = cellSize / 3;
  LCDSetLine(centerX - crossSize, centerY - crossSize, centerX + crossSize, centerY + crossSize, BLACK);
  LCDSetLine(centerX - crossSize, centerY + crossSize, centerX + crossSize, centerY - crossSize, BLACK);
}

void drawCircle(int row, int col) {
  int cellSize = 130 / 3;
  int centerX = (2 * col + 1) * cellSize / 2;
  int centerY = (2 * row + 1) * cellSize / 2;
  int circleRadius = cellSize / 3;
  LCDSetCircle(centerX, centerY, circleRadius, BLACK);
}

void drawSquare(int row, int col, int size, int color) {
  int cellSize = 130 / 3;
  int startX = col * cellSize + (cellSize - size) / 2;
  int startY = row * cellSize + (cellSize - size) / 2;
  if (color == 0)
    LCDSetRect(startX, startY, startX + size - 1, startY + size - 1, 0, BLUE);
  else
    LCDSetRect(startX, startY, startX + size - 1, startY + size - 1, 0, WHITE);
}

void drawBoard(char pools[3][3]) // Draw the Tic-Tac-Toe board in the terminal
{
  sendString("\n\r----------------\n\r");

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      sendString("| ");

      if (pools[j][i] == 'x')
        sendString("X ");
      else if (pools[j][i] == 'c')
        sendString("O ");
      else {
        char digit[3];
        digit[0] = '0' + (i * 3 + j + 1);
        digit[1] = ' ';
        digit[2] = '\0';
        sendString(digit);
      }

      if (j == 2)
        sendString("|");
    }

    if (i != 2)
      sendString("\n\r|---|---|---|\n\r");
    else
      sendString("\n\r");
  }

  sendString("----------------\n\r");
}

int checkWinCondition(char pools[3][3]) {
  // Sprawdz wiersze
  for (int i = 0; i < 3; i++) {
    if (pools[i][0] == pools[i][1] && pools[i][1] == pools[i][2] && pools[i][0] != ' ') {
      return 1; // Jest wygrana
    }
  }

  // Sprawdz kolumny
  for (int j = 0; j < 3; j++) {
    if (pools[0][j] == pools[1][j] && pools[1][j] == pools[2][j] && pools[0][j] != ' ') {
      return 1; // Jest wygrana
    }
  }

  // Sprawdz przekztne
  if (pools[0][0] == pools[1][1] && pools[1][1] == pools[2][2] && pools[0][0] != ' ') {
    return 1; // Jest wygrana
  }
  if (pools[0][2] == pools[1][1] && pools[1][1] == pools[2][0] && pools[0][2] != ' ') {
    return 1; // Jest wygrana
  }

  // Brak wygranej
  return 0;
}

// Funkcja sprawdzajzca, czy plansza jest pelna
int isBoardFull(char pools[3][3]) {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (pools[i][j] == ' ')
        return 0; // Plansza nie jest pelna
  return 1; // Plansza jest pelna
}

int main() {
  InitLCD();
  SetContrast(30);
  LCDClearScreen();
  LCDSetLine(43, 0, 43, 130, BLACK);
  LCDSetLine(87, 0, 87, 130, BLACK);
  LCDSetLine(0, 43, 130, 43, BLACK);
  LCDSetLine(0, 87, 130, 87, BLACK);
  int masterClock, baudrate;
  PMC_PCER = PMC_PCER_PIOA | PMC_PCER_US0; 
  PIOA_PDR = PIOA_PDR_P0 | PIOA_PDR_P1; 
  PIOA_ASR = PIOA_ASR_P0 | PIOA_ASR_P1; 
  // schemat 304 i nota od 306

  US0_CR = US0_CR_RSTRX | US0_CR_RSTTX | US0_CR_RXDIS | US0_CR_TXDIS; // reset rx tx
  // strona 333 reset receiver reset transmitter

  // konfiguracja rejestru MR (mode register) usart mode str. 335
  US0_MR = (0x03 << 6) | (0x04 << 9);

  // BRGR ustawianie pr�dko�ci pracy 346
  baudrate = 9600;
  masterClock = ctl_at91sam7_get_mck_frequency(OSCILLATOR_CLOCK_FREQUENCY);
  US0_BRGR = (masterClock / baudrate) / 16;
  US0_CR = US0_CR_TXEN | US0_CR_RXEN;

  PMC_PCER |= 1 << 2;
  PIOA_PER = 1 << 7 | 1 << 8 | 1 << 9 | 1 << 14 | 1 << 15;
  PIOA_ODR = 1 << 7 | 1 << 8 | 1 << 9 | 1 << 14 | 1 << 15;
  char pools[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      pools[i][j] = ' ';
  char turn = 'x';
  int x = 1;
  int y = 1;
  //drawSquare(x, y, 39, 0);
  int crossx = 1;
  int crossy = 1;
  sendString("\nStart gry");
  while (1) {
    drawBoard(pools);
    if (checkWinCondition(pools)) {
      LCDClearScreen();
      if (turn == 'x') {
        LCDPutStr("Kolko wygralo", 10, 20, LARGE, BLACK, WHITE);
        sendString("\nKolka wygralo");
      } else {
        LCDPutStr("Krzyzyk wygral", 10, 20, LARGE, BLACK, WHITE);
        sendString("\nKrzyzyk wygral");
      }
      break;
    } else if (isBoardFull(pools)) {
      LCDClearScreen();
      LCDPutStr("Remis", 10, 20, LARGE, BLACK, WHITE);
      sendString("\nRemis");
      break;
    }

    if (turn == 'x') {
      sendString("\nTeraz joystick");
      x = crossx;
      y = crossy;
      while (turn == 'x') {
        if ((PIOA_PDSR & 1 << 7) == 0) {
          drawSquare(x, y, 39, 1);
          if (x != 0)
            x--;
          timeDelay(200);
        } else if ((PIOA_PDSR & 1 << 14) == 0) {
          drawSquare(x, y, 39, 1);
          if (x != 2)
            x++;
          timeDelay(200);
        } else if ((PIOA_PDSR & 1 << 8) == 0) {
          drawSquare(x, y, 39, 1);
          if (y != 2)
            y++;
          timeDelay(200);
        } else if ((PIOA_PDSR & 1 << 9) == 0) {
          drawSquare(x, y, 39, 1);
          if (y != 0)
            y--;
          timeDelay(200);
        } else if ((PIOA_PDSR & 1 << 15) == 0) {
          if (pools[x][y] == ' ') {
            pools[x][y] = turn;
            drawCross(x, y);
            crossx = x;
            crossy = y;
            turn = 'c';
          }
        }
      }
    } else if (turn == 'c') {
      sendString("\nPodaj ID");
      switch (getChar()) {
      case '1': {
        x = 0;
        y = 0;
        break;
      }
      case '2': {
        x = 1;
        y = 0;
        break;
      }
      case '3': {
        x = 2;
        y = 0;
        break;
      }
      case '4': {
        x = 0;
        y = 1;
        break;
      }
      case '5': {
        x = 1;
        y = 1;
        break;
      }
      case '6': {
        x = 2;
        y = 1;
        break;
      }
      case '7': {
        x = 0;
        y = 2;
        break;
      }
      case '8': {
        x = 1;
        y = 2;
        break;
      }
      case '9': {
        x = 2;
        y = 2;
        break;
      }
      default: {
        sendString("\n\rBledny znak");
        break;
      }
      }

      if (pools[x][y] == ' ') {
        pools[x][y] = turn;
        drawCircle(x, y);
        turn = 'x';
      } else {
        sendString("\n\rPole zajete");
      }
      timeDelay(200);
    }
  }
}