# 🧬 Genetic Compression
A small experiment with the compression of very short texts. It takes in a training dataset and outputs a compression model. The idea is that using C would make porting to CUDA easier later. This is the second draft of my first serious C program! 🚀

## 🤔 Why Do I Care About This Type of Compression?
In NLP, there was a recent advancement about using compression as a latent space [📄](https://arxiv.org/abs/2212.09410). My hope is that we can use a trainable compressor to get a mini latent space for a transformer to work from. The basic premise is to cut the text into small chunks, call the compressor on them, then extract around 3 bytes to have an encoding scheme. 🎯

## 📏 Scope
I am mainly just experimenting with the compression itself. I may end up using it for something or maybe I never touch it ever again. Additionally, this is my first ever serious C project 👩‍💻, so I apologize in advance for anyone who has the displeasure of trying to figure out my undoubtedly terrible code. 🙈

## 🎯 Target Architecture
I'm specifically targeting the x86 architecture 🖥, primarily because that's the computer I have. I'm just crossing my fingers 🤞 that it ports well to CUDA. 

## 📜 Development History

Strap in for a rollercoaster of trial, error, and aha-moments:

1. **📦 The C++ Byte Affair**: Launched with C++ and bytes, aiming for what felt like the computer's love language. Turns out, it was more of a fling than a committed relationship. This iteration never saw the light of GitHub. 🤷‍♀️

2. **🔗 Bit Buffers & C**: Realizing that bytes were the wrong bait, I pivoted to a linked list approach for bit buffers. I was gunning for that elusive compression magic, but instead stumbled into bug city. 🎉

3. **🛠 C & The Array Adventure**: Taking lessons from my previous detours, I doubled down on C but chose arrays as my trusty sidekick. Enter segfaults, C's way of saying 'Welcome to the big leagues!' 💁‍♀️

Your thoughts and feedback on my ongoing array escapade would be invaluable. Here's hoping this is the iteration that sticks! 🍀
