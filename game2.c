#include <stdlib.h>
#include <string.h>
#include "neslib.h"
#include <nes.h>

#include "level.h"
#include "title.h"
#include "gameover.h"

//#link "chr_generic.s"

#define DEF_METASPRITE_2x2(name, code, pal) \
  const unsigned char name[] = {            \
      0, 0, (code) + 0, pal,                \
      0, 8, (code) + 1, pal,                \
      8, 0, (code) + 2, pal,                \
      8, 8, (code) + 3, pal,                \
      128};

DEF_METASPRITE_2x2(player, 0xd8, 0);

unsigned char collision = 0;

const unsigned char *const playerRunSeq = player;

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = {
    0x0D, // screen color

    0x0D, 0x30, 0x27, 0x00, // background palette 0
    0x1C, 0x20, 0x2C, 0x00, // background palette 1
    0x00, 0x10, 0x20, 0x00, // background palette 2
    0x06, 0x16, 0x26, 0x00, // background palette 3

    0x01, 0x30, 0x2C, 0x00, // sprite palette 0
    0x00, 0x37, 0x25, 0x00, // sprite palette 1
    0x0D, 0x2D, 0x3A, 0x00, // sprite palette 2
    0x0D, 0x27, 0x2A        // sprite palette 3
};

void draw_screen(const char *nametable)
{
  ppu_off();

  vram_adr(NAMETABLE_A);
  vram_unrle(nametable);

  ppu_on_all();
}

void fade_in()
{
  byte vb;
  for (vb = 0; vb <= 4; vb++)
  {
    // set virtual bright value
    pal_bright(vb);
    // wait for 4/60 sec
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
  }
}

void fade_out()
{
  byte vb;
  for (vb = 5; vb > 0; vb--)
  {
    // set virtual bright value
    pal_bright(vb);
    // wait for 4/60 sec
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
  }
}

void title_screen()
{
  char pad;
  int i;

  draw_screen(title);
  fade_in();
  // wait on title screen until button press
  while (1)
  {
    pad = pad_trigger(i);

    if (pad)
    {
      fade_out();
      break;
    }
  }
}

// setup PPU and tables
void setup_graphics()
{

  ppu_off();

  // clear sprites
  oam_hide_rest(0);
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}

// number of actors (4 h/w sprites each)
#define NUM_ACTORS 1

// actor x/y position
byte actor_x;
byte actor_y;

// actor x/y deltas per frame (signed)
sbyte actor_dx;
sbyte actor_dy;

byte runseq;

void main()
{

  char i;      // actor index
  char oam_id; // sprite ID
  char pad;    // controller flags

  setup_graphics();

  //title_screen();

  draw_screen(level);
  //fade_in();

  //draw_screen(level);
  // initialize actor at these coords
  actor_x = 122;
  actor_y = 143;
  actor_dx = 0;
  actor_dy = 0;

  // loop forever
  while (1)
  {
    // start with OAMid/sprite 0
    oam_id = 0;

    // set player 0/1 velocity based on controller
    for (i = 0; i < 2; i++)
    {
      // poll controller i (0-1)
      pad = pad_poll(i);

      // move actor[i] left/right within screen bounds
      if (pad & PAD_LEFT && actor_x > 8)
      {
        actor_dx = -2;
      }
      else if (pad & PAD_RIGHT && actor_x < 230)
      {
        actor_dx = 2;
      }
      else
      {
        actor_dx = 0;
      }

      // move actor[i] up/down within screen bounds
      if (pad & PAD_UP && actor_y > 8)
      {
        actor_dy = -2;
      }
      else if (pad & PAD_DOWN && actor_y < 206)
      {
        actor_dy = 2;
      }
      else
      {
        actor_dy = 0;
      }
    }

    // same result can be achieved by declaring runseq at the start
    // if only one sprite is being used
    // draw and move all actors
    for (i = 0; i < NUM_ACTORS; i++)
    {
      byte runseq = actor_x & 7;

      oam_id = oam_meta_spr(actor_x, actor_y, oam_id, playerRunSeq);
      actor_x += actor_dx;
      actor_y += actor_dy;
    }

    ppu_wait_frame();
  }
}
