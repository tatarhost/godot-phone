# Godot Phone

Неофициальная Android-сборка редактора Godot Engine для телефонов.

## Что собирается

- Godot Editor для Android, архитектура **ARM64**.
- APK публикуется в GitHub Actions как артефакт.
- После успешной сборки из ветки `main` обновляется GitHub Pre-release с тегом `nightly`.

## Скачать APK

Открой вкладку **Releases** и выбери **Godot Phone — Nightly Android build**. APK называется `Godot-Phone-Android-arm64.apk`.

Если релиз ещё не появился, открой **Actions → Build Godot Phone (Android)** и проверь результат последнего запуска.

## Сборка

Workflow: `.github/workflows/android-editor.yml`.

Сборка берёт исходный код Godot Engine из официального репозитория, тег `4.7.2-stable`, и собирает Android Editor через SCons. В репозитории Godot Phone хранятся конфигурация сборки и документация; исходники upstream не копируются сюда целиком.

Для запуска вручную: **Actions → Build Godot Phone (Android) → Run workflow**.

## Важно

Это начальная базовая сборка официального Android Editor, а не отдельная переработка интерфейса под маленький экран. Android-редактор Godot пока имеет ограничения по удобству на телефонах; для комфортной работы могут пригодиться Bluetooth-клавиатура и мышь.

Godot Engine распространяется по лицензии MIT. Godot Phone — независимый проект и не является официальным продуктом Godot Engine. См. [лицензию Godot](https://github.com/godotengine/godot/blob/4.7.2-stable/LICENSE.txt) и [официальную страницу Android](https://godotengine.org/download/android/).
