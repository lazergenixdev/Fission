#include "Fission/core.hpp"
#include "miniaudio.h"
#include "../src/embed/Jet.mp3.hpp"
using namespace fission;

#define HALF 0
#define SCALE 2.0
#define OFFSET 0.0

#define M2(A) #A " " #A
#define M3(A) M2(A) " " #A
#define M4(A) M2(A) " " M2(A)
#define M5(A) M3(A) " " M2(A)

enum Animation_Style {
	Animation_Line_Centered,
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

f32 easeOutExpo(f32 x) {
	return x == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * x);
}
f32 easeInExpo(f32 x) {
	return x == 0.0f ? 0.0f : powf(2.0f, 10.0f * x - 10.0f);
}
f32 easeInQuint(f32 x) {
	return x * x * x * x * x;
}

void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

#if HALF
    ASSERT(!ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount/2, NULL));	
	auto frames = (vec2*)pOutput;
	for (int i = frameCount/2 - 1; i >= 0; --i) {
		frames[i*2-1] = frames[i] * 0.2f;
		frames[i*2]   = frames[i] * 0.2f;
	}
#else
    ASSERT(!ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL));	
	auto frames = (vec2*)pOutput;
	forn (frameCount) {
		frames[i] = frames[i] * 0.3f;
	}
#endif

    (void)pInput;
}

struct Animated_Line
{
	Animation_Style style;
	u32 count;
	c32 characters[100];
	s32 starts[100];
};

bool initialized = false;
bool start_frame = false;
Font font;
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

vec2 add_character(Draw_Data_2d& draw_data, c32 c, vec2 offset, f32 alpha)
{
	Glyph* g;
	auto it = font.glyph_map.find(c);
	if (it == font.glyph_map.end())
		g = &font.fallback;
	else
		g = &it->second;
	draw_data.add_glyph(g, offset, 1.0f, vec4(vec3(4.0f),alpha));
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
	f32 w = f32(engine.graphics.extent.width);
	f32 h = f32(engine.graphics.extent.height);
	for (auto& ln: lines)
	{
		vec2 offset = (vec2(w,h) - measure_text(ln.characters, ln.count)) * 0.5f;
		forn (ln.count) {
			u32 timestamp = ln.starts[0] + ln.starts[i+1] + 0;
			if (current_timestamp < timestamp) goto end;
			f32 dist = f32(current_timestamp - timestamp) / 1000.0f;
			dist = clamp(dist, 0.0f, 1.0f);
			f32 movein  = 30.0f * (1.0f - easeOutExpo(math::min(dist*3.0f, 1.0f)));
			f32 moveout = 30.0f * (easeInQuint(math::max(dist*3.0f-2.0f, 0.0f)));
			offset += add_character(draw_data, ln.characters[i], offset + vec2(0.f, movein - moveout), 1.0f-dist);
		}
		offset.y += 100.0f;
		offset.x = 0.0f;
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
	char *buffer;
	long s;
	FILE *fh = fopen(path, "rb");
	if ( fh != NULL )
	{
		fseek(fh, 0L, SEEK_END);
		s = ftell(fh);
		rewind(fh);
		buffer = arena.alloc<char>(s);
		if ( buffer != NULL )
		{
			fread(buffer, s, 1, fh);
			// we can now close the file
			fclose(fh); fh = NULL;
		}
		if (fh != NULL) fclose(fh);
	}
	return string(buffer, s);
}

void load_animation_data()
{
	lyrics_arena.reset();
	string lyrics_data = read_entire_file(lyrics_arena, "examples/lyrics_data.txt");
	bool cmd_mode = false;
	u32 offset = 0;
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

		}
		if (c == U'\n') {
			cmd_mode = !cmd_mode;
			if (!cmd_mode) {
				if (line.count != offset)
					log::error("incorrect number of timings on line ", lines.count, " (", line.count, ", ", offset, ")");
				lines.count += 1;
			}
			offset = 0;
			continue;
		}

		if (cmd_mode)
		{
			if (c == ' ') {
				offset += 1;
				continue;
			}

			line.starts[offset] *= 10;
			line.starts[offset] += c - '0';
		}
		else
		{
			line.characters[line.count] = c;
			line.count += 1;
		}
	}
}

void on_update(f64 dt, array<Event> events, Render_Context const& ctx)
{
#if HALF
	t += dt * 0.5f;
#else
	t += dt;
#endif
	if (start_frame) t = 0, start_frame = false;

	// Not sure how I want to handle initialization
	if (!initialized)
	{
		arena.create(10_MiB);
		lyrics_arena.create(10_KiB);
		string message {
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
			"너무 바쁜 스케줄여기 번쩍 저기 찰칵"
			"Just Like A Butterfly!"
			"Let's Fly High! Fly!"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"따라라라랄라라 따라라 라랄라랄라"
			"Boy, We Fly Higher & Faster"
			"This Only Gets Better"
			"따라라라랄라라 따라라 라랄라랄라"
			"Boy, We Fly Higher & Faster"
			"This Gunna Be So Killa"
			"Make U Lose Control"
			"구름 속 헤치고"
			"썬더빔 속으로"
			"이제 넌 무시무시한 내 최면에"
			"걸렸어 낚였어 로보트가 됐어"
			"뽀얗다 미쳤다 흠뻑 빠졌는데"
			"사실 난 쉽지 않단 걸 알아야 돼"
			"넌 약간 긴장 설레니?"
			"내 핑크빛 볼 보다 빨개졌어"
			"나는 서울 도쿄 파리 뉴욕"
			"너무 바쁜 스케줄"
			"여기 번쩍 저기 찰칵"
			"Just Like A Butterfly!"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"다 너무나 쉽게 사랑에 빠져들어가"
			"나만의 그 느낌 너를 찾았어 이제야"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"따라라라랄라라 따라라 라랄라랄라"
		};
		Font::Create_Info font_info {
			.font_size = 100,
			.codepoints = decode_utf8(arena, message),
		};
		font.create(font_info);
		arena.reset();

		lines.data = arena.alloc<Animated_Line>(100);
		load_animation_data();

		const void* mp3_data;
		size_t mp3_size;
		load_mp3(&mp3_data, &mp3_size);
		ASSERT(!ma_decoder_init_memory(mp3_data, mp3_size, NULL, &decoder));

		ma_resampler_config config = ma_resampler_config_init(
			decoder.outputFormat,
			decoder.outputChannels,
			decoder.outputSampleRate,
			decoder.outputSampleRate,
			ma_resample_algorithm_linear);
			
		ma_resampler_set_rate(&resampler, decoder.outputSampleRate, decoder.outputSampleRate);

		ma_result result = ma_resampler_init(&config, NULL, &resampler);
		
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
	for (auto const& e: events)
	{
		switch (e.type)
		{
		case Event_Key_Down:
			if (e.key_down.key_id == 0)
			{
				t = f32(engine.window.mouse_position.x)
				  / f32(engine.graphics.extent.width);
				t = clamp(t, 0.0f, 1.0f);
				t *= f32(second_count/SCALE);
				ASSERT(!ma_device_stop(&device));
				ASSERT(!ma_decoder_seek_to_pcm_frame(&decoder, frame_from_seconds(t)));
				ASSERT(!ma_device_start(&device));
				log::info("t = ", t);
			}
			if (e.key_down.key_id == key::R)
			{
				load_animation_data();
				t = 0.0f;
				ma_decoder_seek_to_pcm_frame(&decoder, 0);
			}
			break;
		
		default:
			break;
		}
	}

	vkCmdBindDescriptorSets(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		engine.pipeline_layout, 0, 1, &font.set, 0, nullptr);

	f32 widt = (f32)engine.graphics.extent.width;
	f32 heig = (f32)engine.graphics.extent.height;
	
	if (0)
	{
		engine.draw_data.add_rect_textured({0.0f, heig, 0.0f, heig}, {0.0f, 1.0f, 0.0f, 1.0f}, vec4(1.0f,1.0f,1.0f,0.2f));
	}
	
	draw(engine.draw_data);
	engine.renderer.draw(ctx);
	
	engine.draw_data.push_index(engine.draw_data.current.vertex_count);
	engine.draw_data.push_index(engine.draw_data.current.vertex_count+1);
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE))*widt, 0.0f}, vec2(-1.0f), vec4(1.0f)});
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE))*widt, heig}, vec2(-1.0f), vec4(1.0f)});
	engine.line_renderer.draw(ctx);
}

fission::App_Info::App_Info():
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