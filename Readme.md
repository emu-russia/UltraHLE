# UltraHLE

UltraHLE - классический эмулятор Nintendo 64. Шедевр.

Исходники взяты отсюда: https://code.google.com/archive/p/ultrahle/downloads

Приведены в порядок для сборки под Visual Studio 2022.

## Структура директорий

- src: оригинальные слегка модифцированные исходники
- Build: сюда будет собираться исполняемый файл
- Scripts: проект для VS2022, который ссылками тянет исходники и всё остальное из оригинальной папки src

## Как модифицированы исходники

- Добавлен макрос `_CRT_SECURE_NO_WARNINGS` для unsafe вызовов
- `stricmp` заменена на `_stricmp`
- `memicmp` заменена на `_memicmp`
- `strlwr` заменена на `_strlwr`
- Unicode: Not Set (UltraHLE не использует Unicode)
- Незначительные исправления в UltraHLE.rc и RESOURCE.H

## Glide

UltraHLE требуется устаревший графический API Glide 2.0.

Враппер можно взять тут: http://www.zeckensack.de/glide/
