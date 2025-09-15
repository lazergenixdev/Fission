#include "Fission/core.hpp"
#include "miniaudio.h"
#include "../src/embed/Jet.mp3.hpp"
using namespace fission;

#define HALF 0
#define SCALE 2.0
#define OFFSET 0.0

#ifndef GLFW_KEY_R
#define GLFW_KEY_R 100
#endif

#define M2(A) #A " " #A
#define M3(A) M2(A) " " #A
#define M4(A) M2(A) " " M2(A)
#define M5(A) M3(A) " " M2(A)

#define MAKE_U_LOSE_CONTROL(START) \
	#START " " M4(500) " 0 650 0 " M4(850) " 0 " M3(1400) " " M4(1800) "\n"

#define 구름속헤치고(START) \
	#START " 0 100 0 400 0 900 1150 1400\n"

#define 썬더빔속으로(START) \
	#START " 100 300 550 0 1050 1300 1500\n"

#define 이제넌무시무시한내최면에(START) \
	#START " 0 300 0 550 0 1000 1200 1550 1900 2150 0 2400 0 2600 2800 3050\n"

#define 걸렸어낚였어로보트가됐어(START) \
	#START " 0 150 550 0 900 1150 1400 0 1800 2000 2300 2500 0 2800 3050\n"

const c32 lyrics_data[] = {
   U"Make U Lose Control\n" MAKE_U_LOSE_CONTROL(5800)
	"Make U Lose Control\n" MAKE_U_LOSE_CONTROL(7650)
	"Make U Lose Control\n" MAKE_U_LOSE_CONTROL(9500)
	"Make U Lose Control\n" MAKE_U_LOSE_CONTROL(11350)
	"구름 속 헤치고\n"         구름속헤치고(13700)
	"구름 속 헤치고\n"         구름속헤치고(15550)
	"썬더빔 속으로\n"          썬더빔속으로(17200)
	"썬더빔 속으 \n"           썬더빔속으로(19100)
	"이제 넌 무시무시한 내 최면에\n" 이제넌무시무시한내최면에(20500)
	"걸렸어 낚였어 로보트가 됐어\n" 걸렸어낚였어로보트가됐어(24250)
	"귀엽다 예쁘다 흠뻑 빠졌 는데\n" "27900 0 200 400 0 850 1050 1350 0 1700 1900 0 2200 2400 0 2600 2900\n"
	"사실 난 쉽지 않단 걸 알 아야 돼\n" "31500 0 200 0 500 0 900 1150 0 1400 1700 0 2000 0 2300 0 2550 2800 0 3050\n"
	"\n\n"
	"예쁘장한 나 귀여워?\n" "35600 0 400 650 1000 0 1300 0 1800 2200 2500 0\n"
	"통통한 다리 나 애기 같데\n" "39000 0\n"
	"나는 서울 도쿄 파리 뉴욕\n" "42000 0\n"
	"너무 바쁜 스케줄여기 번쩍 저기 찰칵\n" "45000 0\n"
	"Just Like A Butterfly!\n" "48500 0\n"
	"Let's Fly High! Fly!\n" "49900 0\n"
	"별나라 갈래 파란\n" "51500 0\n"
	"제트기를 타자 파일럿\n" "54000 0\n"
	"Oh My God 맙소사 너에게 빠져들어\n" "57000 0\n"
	"자꾸 빠져들어 나 Automatically Wow\n" "61000 0\n"
	"따라라라랄라라 따라라 라랄라랄라\n" "65000 0\n"
	"Boy, We Fly High Fasta\n" "69000 0\n"
	"This Only Gets Betta\n" "70500 0\n"
	"따라라라랄라라 따라라 라랄라랄라\n" "72500 0\n"
	"Boy, We Fly High Fasta\n" "76500 0\n"
	"This Gunna Be So Killa\n" "78000 0\n"
	"$\n"
	"Make U Lose Control\n"
	"Make U Lose Control\n"
	"Make U Lose Control\n"
	"Make U Lose Control\n"
	"구름 속 헤치고\n"
	"구름 속 헤치고\n"
	"썬더빔 속으로\n"
	"썬더빔 속으로\n"
	"이제 넌 무시무시한 내 최면에\n"
	"걸렸어 낚였어 로보트가 됐어\n"
	"뽀얗다 미쳤다 흠뻑 빠졌는데\n"
	"사실 난 쉽지 않단 걸 알아야 돼\n"
	"넌 약간 긴장 설레니?\n"
	"내 핑크빛 볼 보다 빨개졌어\n"
	"나는 서울 도쿄 파리 뉴욕\n"
	"너무 바쁜 스케줄\n"
	"여기 번쩍 저기 찰칵\n"
	"Just Like A Butterfly!\n"
	"Let's Fly High! Fly!\n"
	"별나라 갈래 파란\n"
	"제트기를 타자 파일럿\n"
	"Oh My God 맙소사 너에게 빠져들어\n"
	"자꾸 빠져들어 나 Automatically\n"
	"다 너무나 쉽게 사랑에 빠져들어가\n"
	"나만의 그 느낌 너를 찾았어 이제야\n"
	"Let's Fly High! Fly!\n"
	"별나라 갈래 파란\n"
	"제트기를 타자 파일럿\n"
	"Oh My God 맙소사 너에게 빠져들어\n"
	"자꾸 빠져들어 나 Automatically\n"
	"Let's Fly High! Fly!\n"
	"별나라 갈래 파란\n"
	"제트기를 타자 파일럿\n"
	"Oh My God 맙소사 너에게 빠져들어\n"
	"자꾸 빠져들어 나 Automatically Wow\n"
	"따라라라랄라라 따라라 라랄라랄라\n"
	"따라라라랄라라 따라라 라랄라랄라\n"
	"따라라라랄라라\n"
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

void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

#if HALF
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount/2, NULL);	
	auto frames = (vec2*)pOutput;
	for (int i = frameCount/2 - 1; i >= 0; --i) {
		frames[i*2-1] = frames[i] * 0.2f;
		frames[i*2]   = frames[i] * 0.2f;
	}
#else
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);	
	auto frames = (vec2*)pOutput;
	forn (frameCount) {
		frames[i] = frames[i] * 0.3f;
	}
#endif

    (void)pInput;
}

struct Animated_Line
{
	u32 count;
	c32 characters[100];
	u32 starts[100];
};

bool initialized = false;
bool start_frame = false;
Font font;
array<Animated_Line> lines;
Arena arena;
f32 t = 0.0f;
ma_decoder decoder;
ma_device device;
ma_uint64 frame_count;
double sample_rate;
double second_count;

uint64_t frame_from_seconds(f32 seconds)
{
	return u64(seconds * sample_rate);
}

void add_character(Draw_Data_2d& draw_data, c32 c, vec2& offset)
{
	Glyph* g;
	auto it = font.glyph_map.find(c);
	if (it == font.glyph_map.end())
		g = &font.fallback;
	else
		g = &it->second;
	draw_data.add_glyph(g, offset, 1.0f, rgba8(255,255,255));
	offset.x += g->advance;
}

void draw(Draw_Data_2d& draw_data)
{
	vec2 offset = vec2(0.0f, -t*37.0f + 500.0f);
	for (auto& ln: lines)
	{
		forn (ln.count) {
			u32 timestamp = ln.starts[0] + ln.starts[i+1] + 0;
			if (u32(t*1000.0f) < timestamp) goto end;
			add_character(draw_data, ln.characters[i], offset);
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

void on_update (f64 dt, array<Event> events, Render_Context const& ctx)
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
		string message {
			"Make U Lose Control"
			"구름 속 헤치고"
			"구름 속 헤치고"
			"썬더빔 속으로"
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
			"Make U Lose Control"
			"Make U Lose Control"
			"Make U Lose Control"
			"구름 속 헤치고"
			"구름 속 헤치고"
			"썬더빔 속으로"
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
			"Let's Fly High! Fly!"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"bridge"
			"다 너무나 쉽게 사랑에 빠져들어가"
			"나만의 그 느낌 너를 찾았어 이제야"
			"Let's Fly High! Fly!"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"Let's Fly High! Fly!"
			"별나라 갈래 파란"
			"제트기를 타자 파일럿"
			"Oh My God 맙소사 너에게 빠져들어"
			"자꾸 빠져들어 나 Automatically Wow"
			"따라라라랄라라 따라라 라랄라랄라"
			"따라라라랄라라 따라라 라랄라랄라"
			"따라라라랄라라"
		};
		Font::Create_Info font_info {
			.font_size = 100,
			.codepoints = decode_utf8(arena, message),
		};
		font.create(font_info);
		arena.reset();

		lines.data = arena.alloc<Animated_Line>(100);
		memset(lines.data, 0, 100 * sizeof(Animated_Line));
		lines.count = 1;

		bool cmd_mode = false;
		u32 offset = 0;
		for (c32 c: lyrics_data)
		{
			auto& line = lines.last();

			if (c == U'$') break;
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

		//ASSERT(!ma_decoder_init_file("Jet.mp3", NULL, &decoder));
		const void* mp3_data;
		size_t mp3_size;
		load_mp3(&mp3_data, &mp3_size);
		ASSERT(!ma_decoder_init_memory(mp3_data, mp3_size, NULL, &decoder));

		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format   = decoder.outputFormat;
		deviceConfig.playback.channels = decoder.outputChannels;
		log::info("channels ", deviceConfig.playback.channels);
		deviceConfig.sampleRate        = decoder.outputSampleRate;
		sample_rate = deviceConfig.sampleRate;
		log::info("sample rate ", sample_rate);
		deviceConfig.dataCallback      = audio_data_callback;
		deviceConfig.pUserData         = &decoder;
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
				ma_decoder_seek_to_pcm_frame(&decoder, frame_from_seconds(t));
				log::info("t = ", t);
			}
			if (e.key_down.key_id == GLFW_KEY_R)
			{
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
	engine.draw_data.add_rect_textured({0.0f, heig, 0.0f, heig}, {0.0f, 1.0f, 0.0f, 1.0f}, rgba8(255,255,255,40));

	draw(engine.draw_data);
	engine.renderer.draw(ctx);
	
	engine.draw_data.push_index(engine.draw_data.current.vertex_count);
	engine.draw_data.push_index(engine.draw_data.current.vertex_count+1);
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE))*widt, 0.0f}, vec2(-1.0f), rgba8(255,255,255)});
	engine.draw_data.push_vertex({{(t / f32(second_count/SCALE))*widt, heig}, vec2(-1.0f), rgba8(255,255,255)});
	engine.line_renderer.draw(ctx);
}

fission::App_Info::App_Info():
    name ( "제트별" )
{}

auto on_create () -> Defaults
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