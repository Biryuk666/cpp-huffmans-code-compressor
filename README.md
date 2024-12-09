# Компрессор/декомпрессор текстового файла на основе кода Хаффмана

## Описание
**Компрессор/декомпрессор** - это инструмент на базе C++, предназначенный для сжатия файла, путем кодирования текста в бинарный код и обратно.

## Возможности
* Для сжатия файла используется метод *CompressDocument()*. На выходе получается файл с типом *"hcc"*.
* Для декодирования данных используется метод *DecompressDocument()*. На выходе получается файл с типом *"txt"*.

## Дальнейшее развитие
* Для ускорения работы **компрессора/декомпрессора** возможна реализация поддержки мультипоточности.