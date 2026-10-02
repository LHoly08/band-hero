from __future__ import annotations

from pathlib import Path
from shutil import copy2
import torch
from demucs.api import Separator, save_audio

class Splitter:
    def __init__(self: Splitter, filename: str, extension: str, path):
        self.__filename = filename
        self.__extension = extension
        self.__path = path

    def split(self: Splitter):
        song: Path = Path(self.getPath())
        
        if not song.is_file():
            raise Exception("File not Found!")
        
        output_dir: Path = Path("Songs") / self.__filename / "Audio"
        output_dir.mkdir(parents=True, exist_ok=True)

        separator: Separator = Separator(
            model="htdemucs_6s",
            device="cuda" if torch.cuda.is_available() else "cpu",
        )

        original, stems = separator.separate_audio_file(str(song))

        main_path = output_dir / "main.mp3"
        if song.suffix.lower() == ".mp3":
            if song.resolve() != main_path.resolve():
                copy2(song, main_path)
        else:
            save_audio(original, str(main_path), samplerate=separator.samplerate)
        print(main_path)

        for name, audio in stems.items():
            path = output_dir / f"{name}.wav"
            save_audio(audio, str(path), samplerate=separator.samplerate)
            print(path)

    def getPath(self: Splitter) -> str:
        return str(Path(self.__path) / (self.__filename + self.__extension))

    __filename: str
    __extension: str
    __path: str
