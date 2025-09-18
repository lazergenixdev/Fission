#include "Fission/core.hpp"
#include "miniaudio.h"
#include "../src/embed/Jet.mp3.hpp"
#include "../src/embed/lyrics_data.txt.hpp"
using namespace fission;

#define HALF 0
#define SCALE 1.0f
#define OFFSET 0.0f

enum Animation_Style {
	Animation_Style_Centered,
	Animation_Style_Moving_Right,
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

f32 easeOutExpo(f32 x) { return x == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * x); }
f32 easeInExpo(f32 x) { return x == 0.0f ? 0.0f : powf(2.0f, 10.0f * x - 10.0f); }
f32 easeInQuad(f32 x) { return x * x; }
f32 easeInQuint(f32 x) { return x * x * x * x * x; }

void audio_data_callback(ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL)
		return;

	ma_result result;
#if HALF
	result = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount/2, NULL);
    ASSERT(!result || result == MA_AT_END);	
	auto frames = (vec2*)pOutput;
	for (int i = frameCount/2 - 1; i >= 0; --i) {
		frames[i*2-1] = frames[i] * 0.2f;
		frames[i*2]   = frames[i] * 0.2f;
	}
#else
    result = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);	
    ASSERT(!result || result == MA_AT_END);	
	auto frames = (vec2*)pOutput;
	forn (frameCount) {
		frames[i] = frames[i] * 0.3f;
	}
#endif
}

struct Animated_Line
{
	u32 count;
	c32 characters[100];
	s32 starts[100];
};

bool initialized = false;
bool start_frame = false;
Font font;
u32 font_size;
array<Animated_Line> lines;
Arena arena;
Arena lyrics_arena;
f32 t = 0.0f;
ma_decoder decoder;
ma_resampler resampler;
ma_device device;
ma_uint64 frame_count;
double sample_rate;
double second_count;

uint64_t frame_from_seconds(f32 seconds)
{
	return u64(seconds * sample_rate);
}

Glyph* lookup_glyph(c32 c)
{
	auto it = font.glyph_map.find(c);
	if (it == font.glyph_map.end())
		return &font.fallback;
	else
		return &it->second;
}

vec2 add_character(Draw_Data_2d& draw_data, c32 c, vec2 offset, f32 scale, f32 alpha)
{
	Glyph* g = lookup_glyph(c);
	draw_data.add_glyph(g, offset, scale, vec4(vec3(8.0f),alpha));
	offset.x += g->advance;
	return vec2(g->advance, 0.0f);
}

vec2 measure_text(c32* text, u32 count)
{
	vec2 box;
	Glyph* g;
	forn (count)
	{
		auto c = text[i];
		auto it = font.glyph_map.find(c);
		if (it == font.glyph_map.end()) g = &font.fallback;
		else                            g = &it->second;
		box.x += g->advance;
		box.y = math::max(box.y, g->rc.bottom() - g->rc.top());
	}
	return box;
}

void draw(Draw_Data_2d& draw_data)
{
	u32 current_timestamp = u32(t*1000.0f);
	auto shader_size = engine.graphics.shader_size();
	f32 w = f32(shader_size.x);
	f32 h = f32(shader_size.y);
	f32 fs = f32(font_size);
	Animation_Style style = Animation_Style_Centered;
	for (auto& ln: lines)
	{
		if (ln.characters[0] == U'L') style = Animation_Style_Moving_Right;
		if (ln.characters[0] == U'M') style = Animation_Style_Centered;
		u32 last_timestamp = ln.starts[0] + ln.starts[ln.count];
		if (current_timestamp > last_timestamp + 3000)
			continue;
		switch (style) {
		default:
		case Animation_Style_Centered: {
			vec2 offset = (vec2(w,h) - measure_text(ln.characters, ln.count)) * 0.5f;
			forn (ln.count) {
				u32 timestamp = ln.starts[0] + ln.starts[i+1] + 0;
				if (current_timestamp < timestamp) goto end;
				f32 dist = f32(current_timestamp - timestamp) / 1000.0f;
				dist = clamp(dist, 0.0f, 1.0f);
				f32 movein  = 0.5f * fs * (1.0f - easeOutExpo(math::min(dist*3.0f, 1.0f)));
				f32 moveout = 0.5f * fs * (easeInQuint(math::max(dist*3.0f-2.0f, 0.0f)));
				f32 scale = (1.0f - easeInQuint(math::max(dist*3.0f-2.0f, 0.0f)));
				offset += add_character(draw_data, ln.characters[i], offset + vec2(0.f, movein - moveout), scale, 1.0f-dist);
			}
		} break;
		case Animation_Style_Moving_Right: {
			forn (ln.count) {
				u32 timestamp = ln.starts[0] + ln.starts[i+1];
				if (current_timestamp < timestamp) break;
			}
			vec2 size = measure_text(ln.characters, ln.count);
			vec2 offset = (vec2(w,h) - measure_text(ln.characters, ln.count)) * 0.5f;
			offset.x += 300.0f - 0.00016f * size.x * (current_timestamp - ln.starts[0]);
			offset.y -= (fs/1500.0f) * f32((s32)current_timestamp - (s32)last_timestamp);
			forn (ln.count) {
				u32 timestamp = ln.starts[0] + ln.starts[i+1];
				if (current_timestamp < timestamp) goto end;
				f32 dist = f32(current_timestamp - timestamp) / 1600.0f;
				f32 dist2 = f32(current_timestamp - timestamp) / 2000.0f;
				if (i == 0 && (ln.characters[0] == U'제' || ln.characters[0] == U'다'))
					dist *= 0.75f, dist2 *= 0.75f;
				if (i == ln.count-1 && ln.characters[ln.count-1] == U'가')
					dist *= 0.5f, dist2 *= 0.5f;
				dist = clamp(dist, 0.0f, 1.0f);
				dist2 = clamp(dist2, 0.0f, 1.0f);
				f32 moveout = 0.5f * fs * (easeInQuint(math::max(dist2*3.0f-2.0f, 0.0f)));
				f32 scale = (1.0f - easeInQuint(math::max(dist*3.0f-2.0f, 0.0f)));
				offset += add_character(draw_data, ln.characters[i], offset + vec2(0.f, -moveout), scale, easeInQuad(1.0f-dist));
			}
		} break;
		}
	}
	end:(void)0;
}

//! TODO: don't embed this
void load_mp3(const void** pdata, size_t* size)
{
	*pdata = embedded::Jet_mp3_start;
	*size = (u8*)embedded::Jet_mp3_end - (u8*)embedded::Jet_mp3_start;
}

//! TODO: move to engine
auto read_entire_file(Arena& arena, const char* path) -> string
{
	char *buffer = 0;
	long s = 0;
	log::info("loading file [", path, "]");
	FILE *fh = fopen(path, "rb");
	log::info("handle ", (void*)fh);
	if ( fh != NULL )
	{
		fseek(fh, 0L, SEEK_END);
		s = ftell(fh);
		rewind(fh);
		buffer = arena.alloc<char>(s);
		fread(buffer, s, 1, fh);
		fclose(fh);
	}
	else
	{
		buffer = (char*)embedded::lyrics_data_txt_start;
		s = long(embedded::lyrics_data_txt_end - embedded::lyrics_data_txt_start);
	}
	return string(buffer, s);
}

bool unicode_equals(c32* s, u32 len, const c32* u)
{
	u32 i = 0;
	while (i < len && s[i] == u[i]) ++i;
	return i == len && u[i] == 0;
}

u32 parse_base10(c32* s, size_t& off)
{
	u32 result = 0;
	while (1)
	{
		u32 d = s[off] - '0';
		if (d >= 10) break;
		result = result * 10 + d;
		off += 1;
	}
	return result;
}

const char* string_from_unicode(const c32* u, size_t len)
{
	static char buf[64];
	forn (len) buf[i] = u[i] < 128 ? char(u[i]) : '?';
	buf[len] = 0;
	return buf;
}

void load_animation_data()
{
	lyrics_arena.reset();
	string lyrics_data = read_entire_file(lyrics_arena, "lyrics_data.txt");
	bool cmd_mode = false;
	u32 offset = 0;
	u32 file_line = 1;
	memset(lines.data, 0, 100 * sizeof(Animated_Line));
	lines.count = 1;
	auto codepoints = decode_utf8(engine.frame_arena, lyrics_data);
	forn (codepoints.count)
	{
		auto c = codepoints.data[i];
		auto& line = lines.last();

		if (c == U'$') break;
		if (c == U'\r') continue;
		if (c == U'#') {
			i += 1;
			u32 cmd_len = 0;
			u32 cmd = i;
			while (i+cmd_len < codepoints.count && codepoints.data[i+cmd_len] != U' ') ++cmd_len;
			i += cmd_len + 1;
			if (unicode_equals(codepoints.data+cmd, cmd_len, U"repeat"))
			{
				u32 index = parse_base10(codepoints.data, i); i += 1;
				u32 timestamp = parse_base10(codepoints.data, i);
				memcpy(lines.data + lines.count - 1, lines.data + index, sizeof(Animated_Line));
				line.starts[0] = timestamp;
				lines.count += 1;
			}
			if (unicode_equals(codepoints.data+cmd, cmd_len, U"repeat_n"))
			{
				u32 index = parse_base10(codepoints.data, i); i += 1;
				u32 count = parse_base10(codepoints.data, i); i += 1;
				u32 timestamp = parse_base10(codepoints.data, i);
				memcpy(lines.data + lines.count - 1, lines.data + index, count*sizeof(Animated_Line));
				u32 base = lines.data[index].starts[0];
				forn (count) {
					auto& start = lines.data[lines.count-1+i].starts[0];
					start = start - base + timestamp;
				}
				lines.count += count;
			}
			while (i < codepoints.count && codepoints.data[i] != U'\n') ++i;
			offset = 0;
			file_line += 1;
			cmd_mode = false;
			continue;
		}
		if (c == U'\n') {
			cmd_mode = !cmd_mode;
			if (!cmd_mode) {
				using namespace formatting;
				if (line.count != offset)
					log::error("incorrect number of timings on line ", file_line, " (", line.count, ", ", offset, ")");
				log::info("line ", pad(lines.count-1, 3), " ", string_from_unicode(line.characters, line.count));
				lines.count += 1;
			}
			offset = 0;
			file_line += 1;
			continue;
		}

		if (cmd_mode) {
			if (c == ' ') {
				offset += 1;
				continue;
			}
			line.starts[offset] *= 10;
			line.starts[offset] += c - '0';
		}
		else {
			line.characters[line.count] = c;
			line.count += 1;
		}
	}
}

struct Particle_System
{
	struct Particle
	{
		vec2 pos;
		vec2 vel;
		f32  vis;
	};

	Particle particles[100];
	f32 spawn_cooldown;

	Particle generate_particle()
	{
		auto sq = [](auto x) { return x*x; };
		return Particle {
			.pos = { sq(random_f32()) * f32(engine.graphics.extent.width),
			         random_f32() * f32(engine.graphics.extent.height) },
			.vel = {0.0f, random_f32() * 100.0f - 50.0f},
			.vis = 0.0f,
		};
	}

	void update(f32 dt)
	{
		rf32 rect = {
			0.0f, f32(engine.graphics.extent.width),
			0.0f, f32(engine.graphics.extent.height),
		};
		spawn_cooldown += 4.0f * dt;
		for (auto&& p: particles)
		{
			if (p.vis == -1.0f || !rect[p.pos]) {
				p.vis = -1.0f;

				//if (spawn_cooldown >= 1.0f)
				{
					p = generate_particle();
					spawn_cooldown = 0.0f;
					continue;
				}
			}
			p.vel.x += (100.0f) * dt;
			p.pos += p.vel * dt;
			p.vis = math::min(p.vis + 0.5f*dt, 1.0);
			engine.draw_data.add_rect(rf32::from_center(p.pos, 2.0f, 2.0f), vec4(10000.0f, 10000.0f, 10000.0f, 0.1f*p.vis));
		}
	}
};
//particle_system {};

void create_font()
{
	string set {
		"Make U Lose Control"
		"구름 속 헤치고"
		"썬더빔 속으로"
		"이제 넌 무시무시한 내 최면에"
		"걸렸어 낚였어 로보트가 됐어"
		"귀엽다 예쁘다 흠뻑 빠졌 는데"
		"사실 난 쉽지 않단 걸 알 아야 돼"
		"예쁘장한 나 귀여워?"
		"통통한 다리 나 애기 같데"
		"나는 서울 도쿄 파리 뉴욕"
		"너무 바쁜 스케줄 여기 번쩍 저기 찰칵"
		"Just Like A Butterfly!"
		"Let's Fly High! Fly!"
		"별나라 갈래 파란"
		"제트기를 타자 파일럿"
		"Oh My God 맙소사 너에게 빠져들어"
		"자꾸 빠져들어 나 Automatically"
		"따라라라랄라라 따라라 라랄라랄라"
		"Boy, We Fly High Fasta"
		"This Only Gets Better"
		"This Gunna Be So Killa"
		"뽀얗다 미쳤다 흠뻑 빠졌는데"
		"사실 난 쉽지 않단 걸 알아야 돼"
		"넌 약간 긴장 설레니?"
		"내 핑크빛 볼 보다 빨개졌어"
		"다 너무나 쉽게 사랑에 빠져들어가"
		"나만의 그 느낌 너를 찾았어 이제야"
	};
	u32 u = math::min(engine.graphics.extent.width, engine.graphics.extent.height);
	font_size = u / 8;
	Font::Create_Info font_info {
		.font_size = int(font_size),
		.codepoints = decode_utf8(temp_arena(), set),
	};
	font.create(font_info);
}
void fission::Application::on_resize(u32)
{
	font.destroy();
	create_font();
}

void on_update(f64 dt, array<Event> events, Render_Context const& ctx)
{
#if HALF
	t += dt * 0.5f;
#else
	t += dt;
#endif
	if (start_frame) t = 0, start_frame = false;

	if (!initialized)
	{
		arena.create(10_MiB);
		lyrics_arena.create(10_KiB);
		create_font();

		lines.data = arena.alloc<Animated_Line>(100);
		load_animation_data();

		const void* mp3_data;
		size_t mp3_size;
		load_mp3(&mp3_data, &mp3_size);
		ASSERT(!ma_decoder_init_memory(mp3_data, mp3_size, NULL, &decoder));

		//ma_resampler_config config = ma_resampler_config_init(
		//	decoder.outputFormat,
		//	decoder.outputChannels,
		//	decoder.outputSampleRate,
		//	decoder.outputSampleRate,
		//	ma_resample_algorithm_linear);
		//ma_resampler_set_rate(&resampler, decoder.outputSampleRate, decoder.outputSampleRate);
		//ma_result result = ma_resampler_init(&config, NULL, &resampler);
		
		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format   = decoder.outputFormat;
		deviceConfig.playback.channels = decoder.outputChannels;
		deviceConfig.sampleRate        = decoder.outputSampleRate;
		deviceConfig.dataCallback      = audio_data_callback;
		deviceConfig.pUserData         = &decoder;
		
		sample_rate = deviceConfig.sampleRate;
		log::info("channels ", deviceConfig.playback.channels);
		log::info("sample rate ", sample_rate);

		ASSERT(!ma_device_init(NULL, &deviceConfig, &device));
		ASSERT(!ma_device_start(&device));
		
		ma_decoder_get_length_in_pcm_frames(&decoder, &frame_count);
		second_count = f64(frame_count) / f64(deviceConfig.sampleRate);
		log::info("seconds ", second_count);

		initialized = true;
		start_frame = true;
	}
	local_persist bool hold = false;
	local_persist v2s32 last_mouse_position;
	for (auto const& e: events)
	{
		switch (e.type)
		{
		case Event_Key_Down:
			if (e.key_down.key_id == key::Mouse_Primary)
			{
				hold = true;
			}
			if (e.key_down.key_id == key::R)
			{
				load_animation_data();
				t = 0.0f;
				ma_decoder_seek_to_pcm_frame(&decoder, 0);
			}
			break;
		case Event_Key_Up:
			if (e.key_down.key_id == key::Mouse_Primary)
			{
				hold = false;
			}
			break;
		default:
			break;
		}
	}
	if (hold && (last_mouse_position != engine.window.mouse_position)) {
	#if !defined(OS_ANDROID)
		t = f32(engine.window.mouse_position.x)
			/ f32(engine.graphics.extent.width);
		t += OFFSET;
		t /= f32(SCALE);
		t = clamp(t, 0.0f, 1.0f);
		t *= f32(second_count);
		ASSERT(!ma_device_stop(&device));
		ASSERT(!ma_decoder_seek_to_pcm_frame(&decoder, frame_from_seconds(t)));
		ASSERT(!ma_device_start(&device));
		engine.last_ticks = ticks();

		log::info("t = ", t);
		last_mouse_position = engine.window.mouse_position;
	#endif
		hold = false;
	}

	vkCmdBindDescriptorSets(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		engine.pipeline_layout, 0, 1, &font.set, 0, nullptr);

	f32 widt = (f32)engine.graphics.shader_size().x;
	f32 heig = (f32)engine.graphics.shader_size().y;
	
	if (0)
	{
		engine.draw_data.add_rect_textured({0.0f, heig, 0.0f, heig}, {0.0f, 1.0f, 0.0f, 1.0f}, vec4(1.0f,1.0f,1.0f,0.2f));
	}
	
	draw(engine.draw_data);
	engine.renderer.draw(ctx);
	
	engine.draw_data.push_index(engine.draw_data.current.vertex_count);
	engine.draw_data.push_index(engine.draw_data.current.vertex_count+1);
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE) - OFFSET)*widt, 0.0f}, vec2(-1.0f), vec4(0.3f)});
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE) - OFFSET)*widt, heig}, vec2(-1.0f), vec4(0.3f)});
	engine.line_renderer.draw(ctx);
}

fission::Application::Application():
    name ( "제트별" )
{}

auto on_create() -> Defaults
{
    return {
        .window_title = "제트별",
        .window_width  = 1280,
        .window_height =  720,
    };
}

//auto on_create_scene (fs::Scene_Key const& key) -> fs::Scene *
//{
//    (void)key;
//    return new Scene;
//}