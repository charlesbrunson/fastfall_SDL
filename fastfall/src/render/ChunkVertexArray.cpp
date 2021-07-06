#include "fastfall/render/ChunkVertexArray.hpp"

#include "fastfall/util/log.hpp"

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

void ChunkVertexArray::draw(RenderTarget& target, RenderState states) const {
	
	states.texture = m_tex;
	states.transform = Transform::combine(states.transform, Transform{ offset });

	if (use_visible_rect) {

		for (const auto& chunk : m_chunks) {

			Rectf bound{
				Vec2f{(float)chunk.chunk_pos.x * m_chunk_size.x, (float)chunk.chunk_pos.y * m_chunk_size.y} *TILESIZE_F,
				Vec2f{chunk.chunk_size} * TILESIZE_F
			};

			if (bound.intersects(visibility)) {
				target.draw(chunk.tva, states);
			}
		}
	}
	else {
		for (const auto& chunk : m_chunks) {
			target.draw(chunk.tva, states);
		}
	}
}

}