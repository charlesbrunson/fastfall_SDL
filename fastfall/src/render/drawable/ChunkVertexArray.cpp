#include "fastfall/render/drawable/ChunkVertexArray.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

namespace ff {

ChunkVertexArray::ChunkVertexArray(Vec2u t_size, Vec2u t_max_chunk_size)
	: m_size(t_size)
	, m_chunk_size(t_max_chunk_size)
{

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


void ChunkVertexArray::set_size(Vec2u size) {

	for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
		bool incr = false;

		if (it->chunk_size.x < m_chunk_size.x
			|| it->chunk_size.y < m_chunk_size.y)
		{

			Vec2u nSize;
			nSize.x = std::min(m_chunk_size.x, m_size.x - m_chunk_size.x * it->chunk_pos.x);
			nSize.y = std::min(m_chunk_size.y, m_size.y - m_chunk_size.y * it->chunk_pos.y);

			if (nSize.x <= 0 || nSize.y <= 0) {
				it = m_chunks.erase(it);
				incr = true;
			}
			else {
				it->tva.resize(nSize);
			}
		}
		if (!incr)
			it++;
	}
	m_size = size;
}

void ChunkVertexArray::do_setTile(Vec2u at, TileID tile) {
	Vec2u chunkPos;
	chunkPos.x = at.x / m_chunk_size.x;
	chunkPos.y = at.y / m_chunk_size.y;

	Vec2u innerPos;
	innerPos.x = at.x % m_chunk_size.x;
	innerPos.y = at.y % m_chunk_size.y;

	Vec2u nSize;
	nSize.x = std::min(m_chunk_size.x, m_size.x - m_chunk_size.x * chunkPos.x);
	nSize.y = std::min(m_chunk_size.y, m_size.y - m_chunk_size.y * chunkPos.y);

	auto iter = std::upper_bound(m_chunks.begin(), m_chunks.end(), chunkPos, [](const Vec2u& pos, const Chunk& chunk) {
		return pos <= chunk.chunk_pos;
		});

	if (iter == m_chunks.end()) {
		m_chunks.emplace_back(Chunk{
			.chunk_pos = chunkPos,
			.chunk_size = nSize,
			.tva = Array{nSize}
			});

		m_chunks.back().tva.setTexture(*m_tex.get());
		m_chunks.back().tva.setTile(innerPos, tile);
		m_chunks.back().tva.offset = Vec2f{ (float)chunkPos.x * m_chunk_size.x, (float)chunkPos.y * m_chunk_size.y } * TILESIZE_F;
	}
	else if (iter->chunk_pos != chunkPos) {
		iter = m_chunks.emplace(iter, Chunk{
			.chunk_pos = chunkPos,
			.chunk_size = nSize,
			.tva = Array{nSize}
			});
		iter->tva.setTexture(*m_tex.get());
		iter->tva.setTile(innerPos, tile);
		iter->tva.offset = Vec2f{ (float)chunkPos.x * m_chunk_size.x, (float)chunkPos.y * m_chunk_size.y } * TILESIZE_F;
	}
	else {
		iter->tva.setTile(innerPos, tile);
	}
}

void ChunkVertexArray::do_blank(Vec2u at) {

	Vec2u chunkPos;
	chunkPos.x = at.x / m_chunk_size.x;
	chunkPos.y = at.y / m_chunk_size.y;

	Vec2u innerPos;
	innerPos.x = at.x % m_chunk_size.x;
	innerPos.y = at.y % m_chunk_size.y;

	auto iter = std::find_if(m_chunks.begin(), m_chunks.end(),
		[chunkPos](const Chunk& chunk) {
			return chunk.chunk_pos == chunkPos;
		});

	if (iter != m_chunks.end() && iter->chunk_pos == chunkPos) {
		iter->tva.blank(innerPos);
	}
}

void ChunkVertexArray::do_clear() {
	m_chunks.clear();
}

void ChunkVertexArray::predraw(predraw_state_t predraw_state) {

	if (predraw_state.updated) {
		while (!commands.empty()) {
			switch (commands.front().type) {
			case Command::Type::Set:
				do_setTile(commands.front().tile_pos, commands.front().tile);
				break;
			case Command::Type::Blank:
				do_blank(commands.front().tile_pos);
				break;
			case Command::Type::Clear:
				do_clear();
				break;
			}
			commands.pop();
		}
	}

	// update draw flags for all chunks
	for (auto& chunk : m_chunks) {
		chunk.draw_flags = DrawFlags::NoDraw;

		Rectf chunk_local{
			scroll + Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} * TILESIZE_F,
			Vec2f{chunk.chunk_size} *TILESIZE_F
		};

		Rectf bounds{ Vec2f{}, Vec2f{m_size} * TILESIZE_F };

		bool in_x = scroll.x == 0.f || chunk_local.left + chunk_local.width <= bounds.left + bounds.width;
		bool in_partial_x = in_x || chunk_local.left < bounds.left + bounds.width;

		bool in_y = scroll.y == 0.f || chunk_local.top + chunk_local.height <= bounds.top + bounds.height;
		bool in_partial_y = in_y || chunk_local.top < bounds.top + bounds.height;

		if (in_partial_x && in_partial_y
			&& (!use_visible_rect || visibility.intersects(getChunkBounds(chunk))))
		{
			chunk.draw_flags = DrawFlags::Draw;
		}
		if (in_partial_x && !in_y
			&& (!use_visible_rect || visibility.intersects(getChunkBounds(chunk, Vec2f{ 0.f, -bounds.height }))))
		{
			chunk.draw_flags |= DrawFlags::DrawOffsetY;
		}
		if (in_partial_y && !in_x
			&& (!use_visible_rect || visibility.intersects(getChunkBounds(chunk, Vec2f{ -bounds.width, 0.f }))))
		{
			chunk.draw_flags |= DrawFlags::DrawOffsetX;
		}
		if (!in_x && !in_y
			&& (!use_visible_rect || visibility.intersects(getChunkBounds(chunk, Vec2f{ -bounds.width, -bounds.height }))))
		{
			chunk.draw_flags |= (DrawFlags::DrawOffsetXY);
		}
	}

	if (debug::enabled(debug::Tilelayer_Chunk) && predraw_state.updated) {
		for (const auto& chunk : m_chunks) {
			if (!debug::repeat((void*)&chunk, offset + scroll)) {

				Rectf bound{
					Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
					Vec2f{chunk.chunk_size} *TILESIZE_F
				};

				auto chunk_box = debug::draw((const void*)&chunk, Primitive::LINE_LOOP, 4, offset + scroll);

				for (auto & i : chunk_box) {
					i.color = Color(255, 0, 0, 200);
				}
				chunk_box[0].pos = math::rect_topleft(bound);
				chunk_box[1].pos = math::rect_topright(bound);
				chunk_box[2].pos = math::rect_botright(bound);
				chunk_box[3].pos = math::rect_botleft(bound);
			}
		}

		if (!debug::repeat((void*)this, offset)) {

			Rectf bound{
				Vec2f{},
				Vec2f{m_size} *TILESIZE_F
			};

			auto size_box = debug::draw((const void*)this, Primitive::LINE_LOOP, 4, offset);

			for (auto & i : size_box) {
				i.color = Color::White;
			}
			size_box[0].pos = math::rect_topleft(bound);
			size_box[1].pos = math::rect_topright(bound);
			size_box[2].pos = math::rect_botright(bound);
			size_box[3].pos = math::rect_botleft(bound);

		}
	}
}


void ChunkVertexArray::draw(RenderTarget& target, RenderState states) const {
	
	states.texture = m_tex;
	states.transform = Transform::combine(states.transform, Transform{ offset + scroll });

	//Vec2f offset{ m_size * TILESIZE_F };
	RenderState shift = states;

	const std::array<glm::fvec2, 4> offsets{
		glm::fvec2{ 0.f, 0.f},
		glm::fvec2{ 0.f, m_size.y * TILESIZE },
		glm::fvec2{ m_size.x * TILESIZE, 0.f },
		glm::fvec2{ m_size.x * TILESIZE, m_size.y * TILESIZE }
	};

	for (const auto& chunk : m_chunks) {

		unsigned off_ndx = 0u;
		for (unsigned flag = chunk.draw_flags; 
			flag > 0u; 
			flag >>= 1, off_ndx++) 
		{

			if (flag & 1) {
				shift.transform = states.transform.translate(-offsets[off_ndx]);
				target.draw(chunk.tva, shift);
			}
		}
	}
}


Rectf ChunkVertexArray::getChunkBounds(const Chunk& chunk, Vec2f draw_offset) const noexcept {
	return Rectf{
		draw_offset + offset + scroll + Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
		Vec2f{chunk.chunk_size} *TILESIZE_F
	};
}

Rectf ChunkVertexArray::getChunkLocalBounds(const Chunk& chunk) const noexcept {
	return Rectf{
			scroll + Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
			Vec2f{chunk.chunk_size} *TILESIZE_F
	};
}

}
