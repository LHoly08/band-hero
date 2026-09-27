from __future__ import annotations

from pathlib import Path
import torch
from demucs.api import Separator, save_audio

class Splitter:
    def __init__(self: Splitter, filename: str, extension: str, path):
        self.__filename = filename
        self.__extension = extension
        self.__path = path

    def split(self: Splitter):
        song: Path = Path(self.getPath())
        
        if not song.exists():
            raise Exception("File not Found!")
        
        output_dir: Path = Path(self.__filename) / "Audio/"
        output_dir.mkdir(parents=True, exist_ok=True)

        separator: Separator = Separator(
            model="htdemucs_6s",
            device="cuda" if torch.cuda.is_available() else "cpu",
        )

        original, stems = separator.separate_audio_file(str(song))

        for name, audio in stems.items():
            path = output_dir / f"{name}.wav"
            save_audio(audio, str(path), samplerate=separator.samplerate)
            print(path)

    def getPath(self: Splitter) -> str:
        return self.__path + self.__filename + self.__extension

    __filename: str
    __extension: str
    __path: str

