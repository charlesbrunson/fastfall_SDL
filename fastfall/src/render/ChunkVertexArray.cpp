#include "fastfall/render/ChunkVertexArray.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

namespace ff {

ChunkVertexArray::ChunkVertexArray(Vec2u t_size, Vec2u t_max_chunk_size)
	: m_size(t_size)
	, m_chunk_size(t_max_chunk_size)
{

}


ChunkVertexArray::ChunkVertexArray(const ChunkVertexArray& rhs)
	: offset(rhs.offset)
	, use_visible_rect(rhs.use_visible_rect)
	, visibility(rhs.visibility)
	, m_size(rhs.m_size)
	, m_chunk_size(rhs.m_chunk_size)
	, m_tex(rhs.m_tex)
	, m_chunks(rhs.m_chunks)
{

}
ChunkVertexArray::ChunkVertexArray(ChunkVertexArray&& rhs)
	: offset(rhs.offset)
	, use_visible_rect(rhs.use_visible_rect)
	, visibility(rhs.visibility)
	, m_size(rhs.m_size)
	, m_chunk_size(rhs.m_chunk_size)
	, m_tex(rhs.m_tex)
	, m_chunks(std::move(rhs.m_chunks))
{

}

ChunkVertexArray& ChunkVertexArray::operator= (const ChunkVertexArray& rhs) {
	offset = rhs.offset;
	use_visible_rect = rhs.use_visible_rect;
	visibility = rhs.visibility;
	m_size = rhs.m_size;
	m_chunk_size = rhs.m_chunk_size;
	m_tex = rhs.m_tex;
	m_chunks = rhs.m_chunks;

	return *this;
}
ChunkVertexArray& ChunkVertexArray::operator= (ChunkVertexArray&& rhs) {
	offset = rhs.offset;
	use_visible_rect = rhs.use_visible_rect;
	visibility = rhs.visibility;
	m_size = rhs.m_size;
	m_chunk_size = rhs.m_chunk_size;
	m_tex = rhs.m_tex;
	m_chunks = std::move(rhs.m_chunks);

	return *this;
}



void ChunkVertexArray::setTexture(const Texture& texture) noexcept {
	m_tex = texture;
	for (auto& chunk : m_chunks) {
		chunk.tva.setTexture(texture);
	}
}

const TextureRef& ChunkVertexArray::getTexture() const noexcept {
	return m_tex;
}

void ChunkVertexArray::setTile(Vec2u at, Vec2u texPos) {
	Vec2u chunkPos;
	chunkPos.x = at.x / m_chunk_size.x;
	chunkPos.y = at.y / m_chunk_size.y;

	Vec2u innerPos;
	innerPos.x = at.x % m_chunk_size.x;
	innerPos.y = at.y % m_chunk_size.y;

	auto iter = std::upper_bound(m_chunks.begin(), m_chunks.end(), chunkPos, [](const Vec2u& pos, const Chunk& chunk) {
		return pos <= chunk.chunk_pos;
		});

	if (iter == m_chunks.end()) {

		Vec2u nSize;
		nSize.x = std::min(m_chunk_size.x, m_size.x - m_chunk_size.x * chunkPos.x);
		nSize.y = std::min(m_chunk_size.y, m_size.y - m_chunk_size.y * chunkPos.y);


		m_chunks.push_back(Chunk{
			.chunk_pos = chunkPos,
			.chunk_size = nSize,
			.tva = TileVertexArray{nSize}
			});

		m_chunks.back().tva.setTexture(*m_tex.get());
		m_chunks.back().tva.setTile(innerPos, texPos);
		m_chunks.back().tva.offset = Vec2f{ (float)chunkPos.x * m_chunk_size.x, (float)chunkPos.y * m_chunk_size.y } * TILESIZE_F;

		//LOG_INFO("add back {}", chunkPos.to_string());
	}
	else if (iter->chunk_pos != chunkPos) {

		Vec2u nSize;
		nSize.x = std::min(m_chunk_size.x, m_size.x - m_chunk_size.x * chunkPos.x);
		nSize.y = std::min(m_chunk_size.y, m_size.y - m_chunk_size.y * chunkPos.y);

		iter = m_chunks.insert(iter, Chunk{
			.chunk_pos = chunkPos,
			.chunk_size = nSize,
			.tva = TileVertexArray{nSize}
			});

		iter->tva.setTexture(*m_tex.get());
		iter->tva.setTile(innerPos, texPos);
		iter->tva.offset = Vec2f{ (float)chunkPos.x * m_chunk_size.x, (float)chunkPos.y * m_chunk_size.y } * TILESIZE_F;

		//LOG_INFO("add insert {}", chunkPos.to_string());
	}
	else {
		iter->tva.setTile(innerPos, texPos);
		//LOG_INFO("set");
	}
}

void ChunkVertexArray::blank(Vec2u at) {

	Vec2u chunkPos;
	chunkPos.x = at.x / m_chunk_size.x;
	chunkPos.y = at.y / m_chunk_size.y;

	Vec2u innerPos;
	innerPos.x = at.x % m_chunk_size.x;
	innerPos.y = at.y % m_chunk_size.y;

	auto iter = std::upper_bound(m_chunks.begin(), m_chunks.end(), chunkPos, [](const Vec2u& pos, const Chunk& chunk) {
		return chunk.chunk_pos <= pos;
		});

	if (iter->chunk_pos == chunkPos) {
		iter->tva.blank(innerPos);
	}
}

void ChunkVertexArray::clear() {
	m_chunks.clear();
}

void ChunkVertexArray::add_scroll(Vec2f scroll_amount) {
	scroll += scroll_amount;

	Vec2f sizef{ m_size.x * TILESIZE_F, m_size.y * TILESIZE_F };

	while (scroll.x < 0.f) scroll.x += sizef.x;
	while (scroll.x >= sizef.x) scroll.x -= sizef.x;

	while (scroll.y < 0.f) scroll.y += sizef.y;
	while (scroll.y >= sizef.y) scroll.y -= sizef.y;

}

void ChunkVertexArray::reset_scroll() {
	scroll = Vec2f{};
}

void ChunkVertexArray::predraw() {

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_CHUNK)) {
		for (const auto& chunk : m_chunks) {
			if (!debug_draw::repeat((void*)&chunk, offset + scroll)) {

				debug_draw::set_offset(offset + scroll);

				Rectf bound{
					Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
					Vec2f{chunk.chunk_size} *TILESIZE_F
				};

				auto& chunk_box = createDebugDrawable<VertexArray, debug_draw::Type::TILELAYER_CHUNK>((const void*)&chunk, Primitive::LINE_LOOP, 4);

				for (int i = 0; i < chunk_box.size(); i++) {
					chunk_box[i].color = Color(255, 0, 0, 200);
				}
				chunk_box[0].pos = math::rect_topleft(bound);
				chunk_box[1].pos = math::rect_topright(bound);
				chunk_box[2].pos = math::rect_botright(bound);
				chunk_box[3].pos = math::rect_botleft(bound);

				debug_draw::set_offset();
			}
		}


		if (!debug_draw::repeat((void*)this, offset)) {

			debug_draw::set_offset(offset);

			Rectf bound{
				Vec2f{},
				Vec2f{m_size} *TILESIZE_F
			};

			auto& size_box = createDebugDrawable<VertexArray, debug_draw::Type::TILELAYER_CHUNK>((const void*)this, Primitive::LINE_LOOP, 4);

			for (int i = 0; i < size_box.size(); i++) {
				size_box[i].color = Color::White;
			}
			size_box[0].pos = math::rect_topleft(bound);
			size_box[1].pos = math::rect_topright(bound);
			size_box[2].pos = math::rect_botright(bound);
			size_box[3].pos = math::rect_botleft(bound);

			debug_draw::set_offset();
		}

	}
}


void ChunkVertexArray::draw(RenderTarget& target, RenderState states) const {
	
	auto draw_offset_chunk = [this](RenderTarget& t_target, RenderState t_states, const Chunk& t_chunk, Vec2f t_offset) {

		if (use_visible_rect) {
			Rectf chunk_bound{
				t_offset + offset + scroll + Vec2f{(float)t_chunk.chunk_pos.x * m_chunk_size.x, (float)t_chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
				Vec2f{t_chunk.chunk_size} *TILESIZE_F
			};

			if (visibility.intersects(chunk_bound)) {
				if (t_offset != Vec2f{}) {
					RenderState mirror = t_states;
					mirror.transform = Transform::combine(t_states.transform, Transform(t_offset));
					t_target.draw(t_chunk.tva, mirror);
				}
				else {
					t_target.draw(t_chunk.tva, t_states);
				}
			}
		}
		else {
			if (t_offset != Vec2f{}) {
				RenderState mirror = t_states;
				mirror.transform = Transform::combine(t_states.transform, Transform(t_offset));
				t_target.draw(t_chunk.tva, mirror);
			}
			else {
				t_target.draw(t_chunk.tva, t_states);
			}
		}
	};

	auto draw_chunk = [this, &draw_offset_chunk](RenderTarget& t_target, RenderState t_states, const Chunk& t_chunk) {

		Rectf chunk_local{
			scroll + Vec2f{(float)t_chunk.chunk_pos.x * m_chunk_size.x, (float)t_chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
			Vec2f{t_chunk.chunk_size} *TILESIZE_F
		};

		Rectf bounds{ Vec2f{}, Vec2f{m_size} *TILESIZE_F };

		bool in_x = scroll.x == 0.f || chunk_local.left + chunk_local.width <= bounds.left + bounds.width;
		bool in_partial_x = in_x || chunk_local.left < bounds.left + bounds.width;

		bool in_y = scroll.y == 0.f || chunk_local.top + chunk_local.height <= bounds.top + bounds.height;
		bool in_partial_y = in_y || chunk_local.top < bounds.top + bounds.height;

		if (in_partial_x && in_partial_y) {
			draw_offset_chunk(t_target, t_states, t_chunk, Vec2f{});
		}
		if (in_partial_x && !in_y) {
			draw_offset_chunk(t_target, t_states, t_chunk, Vec2f{ 0.f, -bounds.height });
		}
		if (in_partial_y && !in_x) {
			draw_offset_chunk(t_target, t_states, t_chunk, Vec2f{ -bounds.width, 0.f });
		}
		if (!in_x && !in_y) {
			draw_offset_chunk(t_target, t_states, t_chunk, Vec2f{ -bounds.width, -bounds.height });
		}
	};

	states.texture = m_tex;
	states.transform = Transform::combine(states.transform, Transform{ offset + scroll });


	for (const auto& chunk : m_chunks) {
		draw_chunk(target, states, chunk);
	}
}

}