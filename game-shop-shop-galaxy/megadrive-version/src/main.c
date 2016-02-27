#include <genesis.h>
#include <gfx.h>
#include "utils.h"
#include "game_shufflepuck.h"

static void game_ShufflePuck();

int main()
{
	game_ShufflePuck();
	return 0;
}

/*	
	Ball game object
*/
struct {
	fix32	inertia;

	fix32	velocity_x,
			velocity_z;

	fix32	initial_pox_x,
			initial_pox_z;

	fix32	pos_x,
			pos_z;

	fix32	prev_pos_x,
			prev_pos_z;

	fix32 	radius;
}ball;

/*
	Player game object
*/
struct {
	fix32	racket_speed;

	fix32	velocity_x,
			velocity_z;

	fix32	initial_pox_x,
			initial_pox_z;

	fix32	pos_x,
			pos_z;

	fix32	prev_pos_x,
			prev_pos_z;

	fix32	width,
			length;
}racket;

/*
	Enemy game object
*/
struct {
	fix32	max_racket_speed;
	fix32	racket_speed;

	fix32	velocity_x,
			velocity_z;

	fix32	initial_pox_x,
			initial_pox_z;

	fix32	pos_x,
			pos_z;

	fix32	prev_pos_x,
			prev_pos_z;

	fix32	width,
			length;
}ai;

static void game_ShufflePuck()
{
	char str[32];	/* debug string */
	u16 vblCount = 0;
	u16 vramIndex = TILE_USERINDEX;
	Sprite sprites[16];

	/* Ball sprite coordinates */
	int ball_2d_x,
		ball_2d_y,
		ball_2d_scale;

	const fix32 persp_coef[] = {fix32DivFloats(1.0, 132.0), fix32DivFloats(5, 132.0), fix32DivFloats(9, 132.0), fix32DivFloats(13, 132.0), fix32DivFloats(17, 132.0), 
								fix32DivFloats(22, 132.0), fix32DivFloats(27, 132.0), fix32DivFloats(34, 132.0), fix32DivFloats(42, 132.0), fix32DivFloats(51, 132.0), 
								fix32DivFloats(64, 132.0), fix32DivFloats(80, 132.0), fix32DivFloats(102, 132.0), fix32DivFloats(132, 132.0)};

	/*	Specific 3D -> 2D Projection */
	static Vect3D_f32 inline project3DTo2D(fix32 x, fix32 z)
	{
		const fix32 top_left_x = FIX32(120);
		const fix32 top_right_x = FIX32(320 - 120);
		const fix32 top_y = FIX32(8);

		const fix32 bottom_left_x = FIX32(0);
		const fix32 bottom_right_x = FIX32(320);
		const fix32 bottom_y = FIX32(130);

		fix32 top_2d_x, bottom_2d_x;

		Vect3D_f32 ret_tuple;

		fix32 norm_x = fix32Add(fix32Div(x, board_width), FIX32(0.5));
		fix32 norm_y = fix32Add(fix32Div(z, board_length), FIX32(0.5));

		norm_y = fix32mapValueToArray(norm_y, FIX32(0.0), FIX32(1.0), persp_coef, 14);
		
		top_2d_x = fix32Add(fix32Mul(fix32InvCoef(norm_x), top_left_x), fix32Mul(norm_x, top_right_x));
		bottom_2d_x = fix32Add(fix32Mul(fix32InvCoef(norm_x), bottom_left_x), fix32Mul(norm_x, bottom_right_x));

		ret_tuple.x = fix32Add(fix32Mul(fix32InvCoef(norm_y), top_2d_x),  fix32Mul(norm_y, bottom_2d_x)); /* proj_2d_x */
		ret_tuple.y = fix32Add(fix32Mul(fix32InvCoef(norm_y), top_y), fix32Mul(norm_y, bottom_y));	/* proj_2d_y */
		ret_tuple.z = fix32RangeAdjust(norm_y, FIX32(0.0), FIX32(1.0), FIX32(0.285), FIX32(1.0));	/* proj_scale */

		return ret_tuple;
	}

	/*	Ball logic */
	void ball_reset(void){
		// global pos_x, pos_z, initial_pox_x, initial_pox_z
		ball.inertia = FIX32(0.1);
		ball.radius = FIX32(0.5);
		ball.initial_pox_x = 0;
		ball.initial_pox_z = 0;
		ball.pos_x = ball.initial_pox_x;
		ball.pos_z = ball.initial_pox_z;
		ball.prev_pos_x = ball.pos_x;
		ball.prev_pos_z = ball.pos_z;
		ball.velocity_x = 0;
		ball.velocity_z = 0;
	}

	void ball_setImpulse(fix32 x, fix32 z){
		ball.velocity_x = x;
		ball.velocity_z = z;
	}

	void ball_bounceX(void){
		ball.velocity_x = fix32Mul(ball.velocity_x, FIX32(-1.0));
	}	

	void ball_bounceZ(void){
		ball.velocity_z = fix32Mul(ball.velocity_z, FIX32(-1.0));
	}

	void ball_setPosition(fix32 x, fix32 z){
		ball.pos_x = x;
		ball.pos_z = z;
	}

	void ball_update(fix32 dt){
		// global pos_x, pos_z, velocity_x, velocity_z, prev_pos_x, prev_pos_z

		/*  Keep track of the ball's previous position */
		ball.prev_pos_x = ball.pos_x;
		ball.prev_pos_z = ball.pos_z;

		/* Move the ball according to its velocity */
		ball.pos_x = fix32Add(ball.pos_x, fix32Mul(ball.velocity_x, dt));
		ball.pos_z = fix32Add(ball.pos_z, fix32Mul(ball.velocity_z, dt));

		/* basic dynamics & collision */
		if (ball.pos_x > fix32Mul(board_width, FIX32(0.5))){
			ball.pos_x = fix32Mul(board_width, FIX32(0.5));
			ball_bounceX();
		}
		else{
			if (ball.pos_x < fix32Mul(board_width, FIX32(-0.5))){
				ball.pos_x = fix32Mul(board_width, FIX32(-0.5));
				ball_bounceX();
			}
		}

		if (ball.pos_z > fix32Mul(board_length, FIX32(0.5))){
			ball.pos_z = fix32Mul(board_length, FIX32(0.5));
			ball_bounceZ();
		}
		else{
			if (ball.pos_z < fix32Mul(board_length, FIX32(-0.5))){
				ball.pos_z = fix32Mul(board_length, FIX32(-0.5));
				ball_bounceZ();
			}
		}

		// if (ball.pos_z > fix32Mul(board_length, FIX32(0.5))){
		// 	ball.pos_x = ball.initial_pox_x;
		// 	ball.pos_z = ball.initial_pox_z;
		// 	ball.prev_pos_x = ball.pos_x;
		// 	ball.prev_pos_z = ball.pos_z;
		// }

		/*	Limit the friction/damping to the areas
			where the puck can be reached by one of the players */
		// if (abs(pos_z) > board_length * 0.25):
		// 	friction_x, friction_z = mulVectorByScalar(velocity_x, velocity_z, -inertia * dt)
		// 	velocity_x, velocity_z = addVectors(velocity_x, velocity_z, friction_x, friction_z)	

		// BMP_drawText("dt = ", 0, 0);
		// fix32ToStr(dt, str, 8);
		// BMP_drawText(str, 6, 0);	

		// fix32ToStr(ball.velocity_x, str, 8);
		// BMP_drawText(str, 0, 1);	
		// fix32ToStr(ball.velocity_z, str, 8);
		// BMP_drawText(str, 10, 1);
		// fix32ToStr(ball.pos_x, str, 8);
		// BMP_drawText(str, 0, 2);	
		// fix32ToStr(ball.pos_z, str, 8);
		// BMP_drawText(str, 10, 2);			
	}

	void gameReset(void){		
		ball_reset();
		ball_setImpulse(FIX32(10.0), FIX32(10.0));
	}

	void renderBall(int ball_2d_x, int ball_2d_y, int ball_2d_scale){
		// ball_2d_y += (240 - 136);

		// intToStr(ball_2d_x, str, 0);
		// BMP_drawText(str, 0, 3);	
		// intToStr(ball_2d_y, str, 0);
		// BMP_drawText(str, 10, 3);
		SPR_setPosition(&sprites[0], ball_2d_x - 12, ball_2d_y + ((224 - 136) - 12));
	}

	void renderPlayer(int player_2d_x, int player_2d_y, int player_2d_scale){
		// render.sprite2d(SCR_MARGIN_X + player_2d_x, player_2d_y - (65 * SCR_SCALE_FACTOR), 64 * SCR_SCALE_FACTOR * player_2d_scale, "@assets/game_racket.png")
	}

	void renderAI(int ai_2d_x, int ai_2d_y, int ai_2d_scale){
		// render.sprite2d(SCR_MARGIN_X + ai_2d_x, ai_2d_y - (65 * SCR_SCALE_FACTOR), 64 * SCR_SCALE_FACTOR * ai_2d_scale, "@assets/game_racket.png")
	}

	u8 ballIsBehindRacket(void){
		if (ball.pos_z < racket.pos_z)
			return TRUE;
		else
			return FALSE;
	}

	u8 BallIsWithinXReach(void){
		if (fix32Add(ball.pos_x, ball.radius) > fix32Sub(racket.pos_x, fix32Mul(racket.width, FIX32(0.5))) 
			&& fix32Sub(ball.pos_x, ball.radius) < fix32Add(racket.pos_x, fix32Mul(racket.width, FIX32(0.5))))
			return TRUE;
		else
			return FALSE;
	}

	u8 BallWasWithinXReach(void){
		if (fix32Add(ball.prev_pos_x, ball.radius) > fix32Sub(racket.prev_pos_x,  fix32Mul(racket.width, FIX32(0.5))) 
			&& fix32Sub(ball.prev_pos_x, ball.radius) < fix32Add(racket.prev_pos_x, fix32Mul(racket.width, FIX32(0.5))))
			return TRUE;
		else
			return FALSE;
	}

	void gameMainLoop(fix32 dt){
		/* Update the ball motion */
		ball_update(dt);

		/* Update the player motion */
		// player_setMouse(mouse_device.GetValue(gs.InputDevice.InputAxisX) / SCR_DISP_WIDTH, mouse_device.GetValue(gs.InputDevice.InputAxisY) / SCR_DISP_HEIGHT);
		// player_update(dt);

		/* Update the AI */
		// ai_updateGameData(ball.pos_x, ball.pos_z, board.board_width, board.board_length);
		// ai_update(dt);

		/* Collisions */
		// if (ball.velocity_z > 0.0)
		// {
		// 	if (!ballIsBehindRacket(ball, player)) && (BallWasWithinXReach(ball, player) or BallIsWithinXReach(ball, player))
		// 	{
		// 		ball.setPosition(ball.pos_x, player.pos_z - ball.velocity_z * dt + min(0.0, player.velocity_z) * dt);
		// 		player.setPosition(player.pos_x, ball.pos_z + player.length);
		// 		ball.bounceZ();
		// 	}
		// }

		/* Compute 3D/2D projections */
		Vect3D_f32 pvect; 
		pvect = project3DTo2D(ball.pos_x, ball.pos_z);
		ball_2d_x = pvect.x;
		ball_2d_y = pvect.y;
		ball_2d_scale = pvect.z;

		// ball_2d_x *= SCR_SCALE_FACTOR
		// ball_2d_y = SCR_DISP_HEIGHT - (ball_2d_y * SCR_SCALE_FACTOR)

		// player_2d_x, player_2d_y, player_2d_scale = project3DTo2D(player.pos_x, player.pos_z, board.board_width, board.board_length)
		// player_2d_x *= SCR_SCALE_FACTOR
		// player_2d_y = SCR_DISP_HEIGHT - (player_2d_y * SCR_SCALE_FACTOR)

		// ai_2d_x, ai_2d_y, ai_2d_scale = project3DTo2D(ai.pos_x, ai.pos_z, board.board_width, board.board_length)
		// ai_2d_x *= SCR_SCALE_FACTOR
		// ai_2d_y = SCR_DISP_HEIGHT - (ai_2d_y * SCR_SCALE_FACTOR)

		// render.clear()
		// render.set_blend_mode2d(render.BlendAlpha)
		// /* Opponent */
		// render.sprite2d(SCR_MARGIN_X + (320 * 0.5) * SCR_SCALE_FACTOR, (SCR_PHYSIC_HEIGHT - 96 * 0.5) * SCR_SCALE_FACTOR, 106 * SCR_SCALE_FACTOR, "@assets/robot5.png")

		// /* Game board */
		// render.image2d(SCR_MARGIN_X, 0, SCR_SCALE_FACTOR, "@assets/game_board.png")

		// /* Score panel */
		// render.image2d(SCR_MARGIN_X, SCR_DISP_HEIGHT - (32 * SCR_SCALE_FACTOR), SCR_SCALE_FACTOR, "@assets/game_score_panel.png")

		/* Render moving items according to their Z position */
		// renderAI(ai_2d_x, ai_2d_y, ai_2d_scale)

		// if (ball.pos_z - ball.radius < player.pos_z + player.length)
		// {
			renderBall(fix32ToInt(ball_2d_x), fix32ToInt(ball_2d_y), ball_2d_scale);
		// 	renderPlayer(player_2d_x, player_2d_y, player_2d_scale)
		// }
		// else
		// {
		// 	renderPlayer(player_2d_x, player_2d_y, player_2d_scale)
		// 	renderBall(ball_2d_x, ball_2d_y, ball_2d_scale)
		// }

		// render.set_blend_mode2d(render.BlendOpaque)
	}

	/*	System stuff */
	SYS_disableInts();

	/* Set a larger tileplan to be able to scroll */
	VDP_setPlanSize(64, 32);
	SPR_init(256);

	VDP_clearPlan(APLAN, 0);
	VDP_clearPlan(BPLAN, 0);

	/* Load the fond tiles */
	VDP_drawImageEx(APLAN, &game_robot_5, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), (320 - 112) / 16, (224 - 112) / 64	, FALSE, FALSE);
	vramIndex += game_robot_5.tileset->numTile;

	VDP_drawImageEx(BPLAN, &game_board, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 0, (224 - 136) / 8, FALSE, FALSE);
	vramIndex += game_board.tileset->numTile;	

	VDP_setPalette(PAL0, game_robot_5.palette->data);
	VDP_setPalette(PAL1, game_board.palette->data);
	VDP_setPalette(PAL2, game_ball.palette->data);

	SPR_initSprite(&sprites[0], &game_ball, 0, 0, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
 //    // SPR_initSprite(&sprites[1], &rse_logo_shadow_alt, 0, 0, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, 0));
	SPR_setPosition(&sprites[0], 64, 64);
 	SPR_update(sprites, 1);	

	SYS_enableInts();

	gameReset();


	while (TRUE)
	{
		VDP_waitVSync();

		// utils_unit_tests();

		gameMainLoop(FIX32(1.0/60.0));
		SPR_update(sprites, 1);	
		// vblCount++;
	}
}
