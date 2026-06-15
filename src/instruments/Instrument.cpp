#include "instruments/Instrument.hpp"

namespace bh {

Note::Note(sf::Vector2f position, std::uint32_t note) noexcept
    : m_position(position), m_note(note) {}

template <>
Instrument<Difficulty::EASY>::Instrument(std::string filename) noexcept
    : m_loader(&Instrument::loadFile, this, "songs/easy/" + filename) {}

template <>
Instrument<Difficulty::HARD>::Instrument(std::string filename) noexcept
    : m_loader(&Instrument::loadFile, this, "songs/hard/" + filename) {}

} // namespace bh