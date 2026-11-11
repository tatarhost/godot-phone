# Godot Phone

Неофициальная сборка полноценного **Godot 4.7.2 Android Editor** для устройств Android ARM64. Цель проекта — дать возможность создавать и редактировать 2D- и 3D-игры непосредственно на телефоне, а не только запускать готовые игры.

## Что входит в базовую сборку

Это именно редактор Godot, собранный из исходного кода движка для Android:

- создание и открытие проектов;
- 2D- и 3D-редакторы;
- сцены, узлы, инспектор и файловая система;
- встроенный редактор GDScript;
- импорт ресурсов и запуск проектов;
- инструменты редактора, которые поддерживает Android-версия Godot.

## Сборка APK

Workflow: `.github/workflows/android-editor.yml`.

1. Загружается исходный код `godotengine/godot`, тег `4.7.2-stable`.
2. SCons компилирует редактор движка для Android ARM64.
3. Gradle собирает полноценный Android Editor APK.
4. APK загружается как артефакт GitHub Actions.
5. Успешная сборка из `main` обновляет prerelease с тегом `nightly`.

Для ручного запуска: **Actions → Build Godot Phone (Android) → Run workflow**.

## Скачать

Открой [Releases](https://github.com/tatarhost/godot-phone/releases) и выбери **Godot Phone — Android ARM64 nightly**. Файл: `Godot-Phone-Android-arm64.apk`.

Если релиз пока отсутствует, проверь вкладку [Actions](https://github.com/tatarhost/godot-phone/actions).

## Ограничения первой версии

Это базовый Android Editor upstream Godot, а не отдельный порт интерфейса Windows/Linux. На сенсорном экране часть действий менее удобна; клавиатура и мышь могут помочь. Сборка Standard не включает C#/.NET-редактор. Следующий этап проекта — собственные улучшения мобильного интерфейса и проверка сборки на реальных устройствах.

## Лицензия и происхождение

Godot Engine распространяется по лицензии MIT. Godot Phone — независимый проект и не является официальным продуктом Godot Engine. Исходники движка не копируются в этот репозиторий: workflow получает их из официального репозитория при сборке.

- [Godot Engine source](https://github.com/godotengine/godot)
- [Godot Android build documentation](https://docs.godotengine.org/en/stable/engine_details/development/compiling/compiling_for_android.html)
- [Godot MIT license](https://github.com/godotengine/godot/blob/4.7.2-stable/LICENSE.txt)
