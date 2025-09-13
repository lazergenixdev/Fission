#include "Fission/core.hpp"
using namespace fission;

#ifndef GLFW_KEY_SPACE
#define GLFW_KEY_SPACE 0
#endif

#ifndef GLFW_KEY_R
#define GLFW_KEY_R 1
#endif

struct Triangle
{
	vec2 p0, p1, p2;
};

f32 random_f32()
{
	return f32(rand()) / f32(RAND_MAX);
}

vec2 rotated(f32 t)
{
	return vec2(sinf(t), cosf(t));
};

f32 clamp(f32 x, f32 min, f32 max)
{
	return x < min ? min : x > max ? max : x;
}

// https://gamedev.stackexchange.com/a/21110
/* Check whether P and Q lie on the same side of line AB */
float Side(vec2 p, vec2 q, vec2 a, vec2 b)
{
    float z1 = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);
    float z2 = (b.x - a.x) * (q.y - a.y) - (q.x - a.x) * (b.y - a.y);
    return z1 * z2;
}

enum {
	NOT_INTERSECTING = 0,
	TOUCHING,
	OVERLAPPING,
	INTERSECTING,
};

/* Check whether segment P0P1 intersects with triangle t0t1t2 */
int Intersecting(vec2 p0, vec2 p1, vec2 t0, vec2 t1, vec2 t2)
{
    /* Check whether segment is outside one of the three half-planes
     * delimited by the triangle. */
    float f1 = Side(p0, t2, t0, t1), f2 = Side(p1, t2, t0, t1);
    float f3 = Side(p0, t0, t1, t2), f4 = Side(p1, t0, t1, t2);
    float f5 = Side(p0, t1, t2, t0), f6 = Side(p1, t1, t2, t0);
    /* Check whether triangle is totally inside one of the two half-planes
     * delimited by the segment. */
    float f7 = Side(t0, t1, p0, p1);
    float f8 = Side(t1, t2, p0, p1);

    /* If segment is strictly outside triangle, or triangle is strictly
     * apart from the line, we're not intersecting */
    if ((f1 < 0 && f2 < 0) || (f3 < 0 && f4 < 0) || (f5 < 0 && f6 < 0)
          || (f7 > 0 && f8 > 0))
        return NOT_INTERSECTING;

    /* If segment is aligned with one of the edges, we're overlapping */
    if ((f1 == 0 && f2 == 0) || (f3 == 0 && f4 == 0) || (f5 == 0 && f6 == 0))
        return OVERLAPPING;

    /* If segment is outside but not strictly, or triangle is apart but
     * not strictly, we're touching */
    if ((f1 <= 0 && f2 <= 0) || (f3 <= 0 && f4 <= 0) || (f5 <= 0 && f6 <= 0)
          || (f7 >= 0 && f8 >= 0))
        return TOUCHING;

    /* If both segment points are strictly inside the triangle, we
     * are not intersecting either */
    if (f1 > 0 && f2 > 0 && f3 > 0 && f4 > 0 && f5 > 0 && f6 > 0)
        return NOT_INTERSECTING;

    /* Otherwise we're intersecting with at least one edge */
    return INTERSECTING;
}

//App_Info::App_Info():
//    version      ( fs::make_compressed_version<0,1,7> ),
//    version_info ( "vanilla/dev" ),
//    name         ( "line-effect" )
//{}

auto on_create () -> Defaults
{
    return {
        .window_title = "네, 한국어 정말 재미있어요! ❤🎄",
        .window_width  = 1280,
        .window_height =  720,
    };
}

enum: u32 {
	MOVE_LEFT  = (1<<0),
	MOVE_RIGHT = (1<<1),
	MOVE_UP    = (1<<2),
	MOVE_DOWN  = (1<<3),
};

struct Player
{
	vec2 pos, vel;
	u32 move;

	void update(f32 dt)
	{
		vec2 dv {};
		if (move & MOVE_LEFT)  dv.x -= 1.0f;
		if (move & MOVE_RIGHT) dv.x += 1.0f;
		if (move & MOVE_UP)    dv.y += 1.0f;
		if (move & MOVE_DOWN)  dv.y -= 1.0f;
		vel += 0.3f * dv * dt;
		vel.x = clamp(vel.x, -0.1f, 0.1f);
		vel.y = clamp(vel.y, -0.1f, 0.1f);
		pos += vel * dt;
	}

	void draw(Draw_Data_2d& draw_data, vec2 offset, vec2 dir)
	{
		draw_data.push_index(draw_data.current.vertex_count);
		draw_data.push_index(draw_data.current.vertex_count+1);
		draw_data.push_index(draw_data.current.vertex_count+1);
		draw_data.push_index(draw_data.current.vertex_count+2);
		draw_data.push_index(draw_data.current.vertex_count+2);
		draw_data.push_index(draw_data.current.vertex_count);

		vec2 n = (dir - pos).norm();
		f32 theta = atan2f(n.x, n.y);

		vec2 A = pos + 0.2f * n;
		vec2 B = pos + 0.2f * rotated(theta + (TAU/3.0));
		vec2 C = pos + 0.2f * rotated(theta + 2.0*(TAU/3.0));

		draw_data.push_vertex({A + offset, {}, rgba8(255,255,255)});
		draw_data.push_vertex({B + offset, {}, rgba8(255,255,255)});
		draw_data.push_vertex({C + offset, {}, rgba8(255,255,255)});
	}
};

struct Bullet
{
	vec2 pos, vel;

	void update(f32 dt)
	{
		pos += vel * dt;
	}

	auto p2() -> vec2 { return pos - 0.1f * vel; }

	void draw(Draw_Data_2d& draw_data, vec2 offset, rgba8 color)
	{
		draw_data.push_index(draw_data.current.vertex_count);
		draw_data.push_index(draw_data.current.vertex_count+1);

		draw_data.push_vertex({offset + pos, {}, color});
		draw_data.push_vertex({offset + p2(), {}, color});
	}
};

struct Enemy
{
	vec2 pos;
	f32 rot, a, b, c;

	void update(f32 dt, vec2 to)
	{
		vec2 dir = (to - pos).norm();
		pos += 0.1f * dir * dt;
	}

	auto tri() -> Triangle {
		return {
			pos + a * rotated(rot),
			pos + b * rotated(rot - (TAU/3.0)),
			pos + c * rotated(rot - 2.0*(TAU/3.0)),
		};
	}

	void draw(Draw_Data_2d& draw_data, vec2 offset)
	{
		draw_data.push_index(draw_data.current.vertex_count);
		draw_data.push_index(draw_data.current.vertex_count+1);
		draw_data.push_index(draw_data.current.vertex_count+1);
		draw_data.push_index(draw_data.current.vertex_count+2);
		draw_data.push_index(draw_data.current.vertex_count+2);
		draw_data.push_index(draw_data.current.vertex_count);

		vec2 A = pos + a * rotated(rot);
		vec2 B = pos + b * rotated(rot - (TAU/3.0));
		vec2 C = pos + c * rotated(rot - 2.0*(TAU/3.0));

		auto color = rgba8(255,255,255);
		draw_data.push_vertex({A + offset, {}, color});
		draw_data.push_vertex({B + offset, {}, color});
		draw_data.push_vertex({C + offset, {}, color});
	}

	static Enemy random()
	{
		f32 t = TAU * random_f32();
		return {
			.pos = rotated(t),
			.a = 0.3f*random_f32()+0.05f, .b = 0.3f*random_f32()+0.05f, .c = 0.3f*random_f32()+0.05f,
		};
	}
};

bool initialized = false;
vec2 view_offset;
Player player;
Enemy enemies[6];
Bullet bullets[100];
u32 bullet_count;

void on_update (f64 dt, array<Event> events, Render_Context const& ctx)
{
	if (!initialized)
	{
		for (auto& enemy: enemies)
		{
			enemy = Enemy::random();
		}
		initialized = true;
	}
//	view_offset = -player.pos;

	auto pos = vec2(engine.window.mouse_position);
	pos /= vec2(engine.graphics.extent.width, engine.graphics.extent.height);
	pos = pos * 2.0f - vec2(1.0f);
	pos.y = -pos.y;
	f32 aspect = f32(engine.graphics.extent.height) / f32(engine.graphics.extent.width);
	pos.x /= aspect;
	
//	pos -= view_offset;

	for (auto const& e: events)
	{
		switch (e.type)
		{
		case Event_Key_Down: {
			switch (e.key_down.key_id)
			{
			case GLFW_KEY_SPACE:
				bullets[bullet_count++] = {
					.pos = player.pos,
					.vel = (pos - player.pos).norm(),
				};
				break;

//			case GLFW_KEY_W: player.move |= MOVE_UP;    break;
//			case GLFW_KEY_S: player.move |= MOVE_DOWN;  break;
//			case GLFW_KEY_A: player.move |= MOVE_LEFT;  break;
//			case GLFW_KEY_D: player.move |= MOVE_RIGHT; break;

			case GLFW_KEY_R: {
				bullet_count = 0;
				for (auto& enemy: enemies)
				{
					enemy = Enemy::random();
				}
			} break;

			default: break;
			}
		} break;

		case Event_Key_Up: {
			switch (e.key_up.key_id)
			{
//			case GLFW_KEY_W: player.move &=~ MOVE_UP;    break;
//			case GLFW_KEY_S: player.move &=~ MOVE_DOWN;  break;
//			case GLFW_KEY_A: player.move &=~ MOVE_LEFT;  break;
//			case GLFW_KEY_D: player.move &=~ MOVE_RIGHT; break;
			default: break;
			}
		} break;
		}
	}

	player.update(f32(dt));
	player.draw(engine.draw_data, view_offset, pos);

	for (auto& enemy: enemies)
	{
		enemy.update(f32(dt), player.pos);
		enemy.draw(engine.draw_data, view_offset);
	}

	for (auto& bullet: array{bullet_count, bullets})
	{
		bullet.update(f32(dt));
		rgba8 color = rgba8(255,255,255);

		vec2 s0 = bullet.pos;
		vec2 s1 = bullet.p2();
		for (auto& enemy: enemies)
		{
			Triangle tri = enemy.tri();
			if (Intersecting(s0, s1, tri.p0, tri.p1, tri.p2) != 0)
			{
				enemy = {};
				bullet = {};
			}
		}

		bullet.draw(engine.draw_data, view_offset, color);
	}

	engine.line_renderer.draw(ctx);
}

//auto on_create_scene (fs::Scene_Key const& key) -> fs::Scene *
//{
//    (void)key;
//    return new Scene;
//}