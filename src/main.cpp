#include <bn_core.h>
#include <bn_display.h>
#include <bn_log.h>
#include <bn_keypad.h>
#include <bn_random.h>
#include <bn_rect.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_size.h>
#include <bn_string.h>
#include <bn_backdrop.h>
#include <bn_color.h>
#include "bn_sprite_items_dot.h"
#include "bn_sprite_items_square.h"
#include "common_fixed_8x16_font.h"

// Pixels / Frame player moves at
static constexpr bn::fixed SPEED = 2; // Changed speed from 1 to 2
static constexpr bn::fixed boost_speed = 4;

// Width and height of the the player and treasure bounding boxes
static constexpr bn::size PLAYER_SIZE = {8, 8};
static constexpr bn::size TREASURE_SIZE = {8, 8};

// Full bounds of the screen
static constexpr int MIN_Y = -bn::display::height() / 2;
static constexpr int MAX_Y = bn::display::height() / 2;
static constexpr int MIN_X = -bn::display::width() / 2;
static constexpr int MAX_X = bn::display::width() / 2;

// Number of characters required to show the longest numer possible in an int (-2147483647)
static constexpr int MAX_SCORE_CHARS = 11;

// Score location
static constexpr int SCORE_X = 70;
static constexpr int SCORE_Y = -70;

// Player starting location
static constexpr int player_start_pos_x = -50;
static constexpr int player_start_pos_y = 0;

// Dot starting position
static constexpr int dot_start_pos_x = 50;
static constexpr int dot_start_pos_y = 0;

// Screen Edges
static constexpr int left_edge = -120;
static constexpr int right_edge = 120;
static constexpr int bottom_edge = 80;
static constexpr int top_edge = -80;

int main()
{
    bn::core::init();

    bn::backdrop::set_color(bn::color(14, 25, 31)); // sky blue backdrop color
    bn::random rng = bn::random();

    // Will hold the sprites for the score
    bn::vector<bn::sprite_ptr, MAX_SCORE_CHARS> score_sprites = {};
    bn::sprite_text_generator text_generator(common::fixed_8x16_sprite_font);

    int score = 0;

    // Speed boost variables
    int boosts = 3;
    bool is_boosting = false;
    int boost_frame_counter = 0;

    bn::sprite_ptr player = bn::sprite_items::square.create_sprite(player_start_pos_x, player_start_pos_y);
    bn::sprite_ptr treasure = bn::sprite_items::dot.create_sprite(dot_start_pos_x, dot_start_pos_y);

    while (true)
    {
        // Handle Boost Activation & Timer
        if (!is_boosting)
        {
            if (bn::keypad::a_pressed() && boosts > 0)
            {
                is_boosting = true;
                boosts--;
                boost_frame_counter = 0; // Reset timer for this boost instance
            }
        }
        else
        {
            // Increment the counter only while actively boosting
            boost_frame_counter++;

            if (boost_frame_counter >= 120)
            {
                is_boosting = false; // Turn off the boost
                boost_frame_counter = 0;
            }
        }

        // Determine player speed for this frame
        bn::fixed current_speed = is_boosting ? boost_speed : SPEED;

        // Move player using the determined speed
        if (bn::keypad::left_held())
        {
            player.set_x(player.x() - current_speed);
        }
        if (bn::keypad::right_held())
        {
            player.set_x(player.x() + current_speed);
        }
        if (bn::keypad::up_held())
        {
            player.set_y(player.y() - current_speed);
        }
        if (bn::keypad::down_held())
        {
            player.set_y(player.y() + current_speed);
        }

        // Send the player to the other side of the screen when they go out of bounds
        if (bn::keypad::right_held() && player.x() == right_edge)
        {
            player.set_x(left_edge);
        }
        if (bn::keypad::left_held() && player.x() == left_edge)
        {
            player.set_x(right_edge);
        }
        if (bn::keypad::up_held() && player.y() == top_edge)
        {
            player.set_y(bottom_edge);
        }
        if (bn::keypad::down_held() && player.y() == bottom_edge)
        {
            player.set_y(top_edge);
        }

        // The bounding boxes of the player and treasure, snapped to integer pixels
        bn::rect player_rect = bn::rect(player.x().round_integer(),
                                        player.y().round_integer(),
                                        PLAYER_SIZE.width(),
                                        PLAYER_SIZE.height());
        bn::rect treasure_rect = bn::rect(treasure.x().round_integer(),
                                          treasure.y().round_integer(),
                                          TREASURE_SIZE.width(),
                                          TREASURE_SIZE.height());

        // If the bounding boxes overlap, set the treasure to a new location an increase score
        if (player_rect.intersects(treasure_rect))
        {
            // Jump to any random point in the screen
            int new_x = rng.get_int(MIN_X, MAX_X);
            int new_y = rng.get_int(MIN_Y, MAX_Y);
            treasure.set_position(new_x, new_y);

            score++;
        }

        // Update score display
        bn::string<MAX_SCORE_CHARS> score_string = bn::to_string<MAX_SCORE_CHARS>(score);
        score_sprites.clear();
        text_generator.generate(SCORE_X, SCORE_Y,
                                score_string,
                                score_sprites);

        // Update RNG seed every frame so we don't get the same sequence of positions every time
        rng.update();

        // If the player hits start the game restarts
        if (bn::keypad::start_pressed()){
            player.set_position(player_start_pos_x, player_start_pos_y);
            treasure.set_position(dot_start_pos_x, dot_start_pos_y);
            boosts = 3;

            score = 0;
        }

        bn::core::update();
    }
}