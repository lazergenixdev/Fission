#include <Fission/Core/Input/Event.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Fission/Core/Renderer_2D.hh>
#include <vector>
#include <random>
#define CELL_SIZE   24

using uint = unsigned int;

#define make_shape(Row1,Row2,Row3,Row4,Row5) \
(fs::u32(Row1) << 20) | \
(fs::u32(Row2) << 15) | \
(fs::u32(Row3) << 10) | \
(fs::u32(Row4) <<  5) | \
(fs::u32(Row5)      )

using Shape = fs::u32;

static constexpr int   shape_count = 6;
static constexpr Shape shape_data[] = {
// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b00000,
		0b00000,
		0b11110,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b00100,
		0b00100,
		0b00100
	),
	make_shape(
		0b00000,
		0b00000,
		0b00000,
		0b11110,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b00100,
		0b00100,
		0b00100
	),

// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b00000,
		0b00100,
		0b01110,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b01100,
		0b00100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00000,
		0b01110,
		0b00100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b00110,
		0b00100,
		0b00000
	),

// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b00000,
		0b00110,
		0b01100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00000,
		0b01100,
		0b00110,
		0b00000
	),
	make_shape(
		0b00000,
		0b01000,
		0b01100,
		0b00100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b01100,
		0b01000,
		0b00000
	),

// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b00110,
		0b00100,
		0b00100,
		0b00000
	),
	make_shape(
		0b00000,
		0b01000,
		0b01110,
		0b00000,
		0b00000
	),
	make_shape(
		0b00000,
		0b00100,
		0b00100,
		0b01100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00000,
		0b01110,
		0b00010,
		0b00000
	),

// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b00100,
		0b00100,
		0b00110,
		0b00000
	),
	make_shape(
		0b00000,
		0b00010,
		0b01110,
		0b00000,
		0b00000
	),
	make_shape(
		0b00000,
		0b01100,
		0b00100,
		0b00100,
		0b00000
	),
	make_shape(
		0b00000,
		0b00000,
		0b01110,
		0b01000,
		0b00000
	),

// // // // // // // // // // // // // // //
	make_shape(
		0b00000,
		0b01100,
		0b01100,
		0b00000,
		0b00000
	),
	make_shape(
		0b00000,
		0b01100,
		0b01100,
		0b00000,
		0b00000
	),
	make_shape(
		0b00000,
		0b01100,
		0b01100,
		0b00000,
		0b00000
	),
	make_shape(
		0b00000,
		0b01100,
		0b01100,
		0b00000,
		0b00000
	),
};

struct Piece {
	fs::v2s32 position;
	fs::rgba8 color;
	fs::u32 shape_index;
	int rotation;

	Piece& add_rotation(int x) { rotation = (rotation + x) & 0b11; return *this; }
};

struct Tetris {
	fs::v2s32 size;
	fs::rgba8* grid = nullptr;
	Piece current_piece;
	Piece ghost_piece;
	bool is_game_over = false;
	int score = 0;

	float cell_size = CELL_SIZE;

	float fall_time  = 0.0f;
	float fall_speed = 0.6f;

	std::mt19937 rng;
	std::uniform_int_distribution<fs::u32> shape_dist{0, shape_count-1};

	void init_piece(Piece& piece) {
        unsigned int col = rng();
		piece.shape_index = shape_dist(rng)*4;
		piece.rotation    = 0;
		piece.position    = { size.x / 2 - 3, 0 };
		piece.color       = std::bit_cast<fs::rgba8>(col);
		piece.color.a = 255;
	}

	bool valid_move(Piece p, fs::v2s32 offset, int rotation) {
		if(rotation) p.add_rotation(rotation);
		p.position += offset;
		Shape shape = shape_data[p.shape_index + p.rotation];

		for (int y = 0; y < 5; ++y) {
			for (int x = 0; x < 5; ++x) {
				auto cell_is_solid = bool((shape >> (y * 5 + x)) & 1);
				if (!cell_is_solid) continue;

				int _x = p.position.x +	x;
				int _y = p.position.y +	y;

				if (_x < 0 || _x >= size.x ||
					_y < 0 || _y >= size.y)
					return false;

				if (grid[_y * size.x + _x] != fs::rgba8{}) {
					return false;
				}
			}
		}

		return true;
	}

	bool all_cells_filled_in_row(int row) {
		for (int x = 0; x < size.x; ++x) {
			if (grid[row * size.x + x] == fs::rgba8{})
				return false;
		}
		return true;
	}

	int clear_lines() {
		int lines_cleared = 0;

		for (int row = 0; row < size.y; ++row) {
			if (all_cells_filled_in_row(row)) {
				++lines_cleared;

				// move above rows down
				memmove(grid + size.x, grid, sizeof(fs::rgba8) * size.x * row);
				memset(grid, 0, sizeof(fs::rgba8) * size.x);
			}
		}

		return lines_cleared;
	}

	void lock_piece(Piece& piece) {
		Shape shape = shape_data[piece.shape_index + piece.rotation];

		// place shape onto grid
		for (int y = 0; y < 5; ++y) {
			for (int x = 0; x < 5; ++x) {
				auto cell_is_solid = bool( (shape >> (y * 5 + x)) & 1 );
				if (cell_is_solid) {
					grid[(piece.position.y + y) * size.x + (piece.position.x + x)] = piece.color;
				}
			}
		}

		// clear lines and update score
		int lines_cleared = clear_lines();
		score += lines_cleared * 100;

		// create new piece
		init_piece(current_piece);

		// check game over
		if (!valid_move(current_piece, {}, 0)) {
			is_game_over = true;
		}
	}

	void update(float dt, fs::Renderer_2D& r) {
		fall_time += dt;
		if (fall_time >= fall_speed) {
			if (!is_game_over) {
				if (valid_move(current_piece, {0, 1}, 0))
					current_piece.position.y += 1;
				else
					lock_piece(current_piece);
			}
			fall_time = 0;
		}
		
		draw(r);

		if (is_game_over) {

		}
	}

	void draw(fs::Renderer_2D& r) {
		fs::v2f32 offset = {300.0f, 10.0f};
		fs::v2f32 cs     = fs::v2f32(cell_size);

		for (int y = 0; y < size.y; ++y) {
			for (int x = 0; x < size.x; ++x) {
				auto color = grid[y * size.x + x];
				if (color != fs::rgba8{}) {
					float _x = cell_size * x;
					float _y = cell_size * y;
					r.add_rect(fs::rf32::from_topleft(_x, _y, cell_size, cell_size)+offset, color);
				}
			}
		}

		auto draw_piece = [&](Piece const& p) {
			Shape shape = shape_data[p.shape_index + p.rotation];
			for (int y = 0; y < 5; ++y) {
				for (int x = 0; x < 5; ++x) {
					auto cell_is_solid = bool((shape >> (y * 5 + x)) & 1);
					if (cell_is_solid) {
						int grid_x = p.position.x + x;
						int grid_y = p.position.y + y;

						float _x = cell_size * grid_x;
						float _y = cell_size * grid_y;

						r.add_rect(fs::rf32::from_topleft(_x, _y, cs)+offset, p.color);
					}
				}
			}
		};

		ghost_piece = current_piece;
		while (valid_move(ghost_piece, {0, 1}, 0))
			ghost_piece.position.y += 1;
		ghost_piece.color = {200, 200, 200, 10};

		draw_piece(ghost_piece);
		draw_piece(current_piece);

	//	auto pos = (fs::v2f32)current_piece.position * cs;
	//	r.add_rect_outline(fs::rf32::from_topleft(pos, cs*5.0f) + offset, fs::colors::Red);

		r.add_rect_outline(fs::rf32::from_topleft((fs::v2f32)size) * cell_size + offset, fs::colors::White);
	}
};

Tetris* tetris_init(int w, int h) {
	Tetris* t = new Tetris;
	auto seed = (uint)time(nullptr);
	t->rng.seed(seed);
	t->size = { w, h };
	t->grid = new fs::rgba8[t->size.x*t->size.y];
	t->init_piece(t->current_piece);
	memset(t->grid, 0, sizeof(fs::rgba8) * t->size.x * t->size.y);
	return t;
}

void tetris_update(Tetris* t, float dt, std::vector<fs::Event> const& events, fs::Renderer_2D& r) {
	for (auto&&e : events) {
		switch (e.type)
		{
		case fs::Event_Key_Down: {
			switch (e.key_down.key_id)
			{
			case fs::keys::A:
				if (t->valid_move(t->current_piece, {-1, 0}, 0)) t->current_piece.position.x -= 1;
			break;

			case fs::keys::D:
				if (t->valid_move(t->current_piece, { 1, 0}, 0)) t->current_piece.position.x += 1;
			break;

			case fs::keys::S:
				if (t->valid_move(t->current_piece, { 0, 1}, 0)) t->current_piece.position.y += 1;
			break;

			case fs::keys::Q: {
				int i = 0;
				fs::v2s32 offset = {};
				while (i++ < 3) {
					if (t->valid_move(t->current_piece, offset, 1)) {
						t->current_piece.add_rotation(1);
						t->current_piece.position += offset;
						break;
					}
                    if(t->current_piece.position.x < (t->size.x / 2))
						offset.x += 1;
                    else
						offset.x -= 1;
                }
				break;
			}

			case fs::keys::E:{
				int i = 0;
				fs::v2s32 offset = {};
				while (i++ < 3) {
					if (t->valid_move(t->current_piece, offset, -1)) {
						t->current_piece.add_rotation(-1);
						t->current_piece.position += offset;
						break;
					}
                    if (t->current_piece.position.x < (t->size.x / 2))
						offset.x += 1;
                    else
						offset.x -= 1;
                }
				break;
			}

			case fs::keys::Space:
				while (t->valid_move(t->current_piece, {0, 1}, 0)) t->current_piece.position.y += 1;
				t->lock_piece(t->current_piece);
			break;

			case fs::keys::Back:
				if (t->is_game_over) {
					t->init_piece(t->current_piece);
					memset(t->grid, 0, sizeof(fs::rgba8) * t->size.x * t->size.y);
					t->is_game_over = false;
				}
			break;

			default:break;
			}
			break;
		}
		default:break;
		}
	}
	
	t->update(dt, r);
}

void tetris_uninit(Tetris* t) {
	delete [] t->grid;
	delete t;
}
