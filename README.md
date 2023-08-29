# 🧬 Genetic Compression
A small experiment with the compression of very short texts. It takes in a training dataset and outputs a compression model. The idea is that using C would make porting to CUDA easier later. This is the second draft of my first serious C program! 🚀

## 🤔 Why Do I Care About This Type of Compression?
In NLP, there was a recent advancement about using compression as a latent space [📄](https://arxiv.org/abs/2212.09410). My hope is that we can use a trainable compressor to get a mini latent space for a transformer to work from. The basic premise is to cut the text into small chunks, call the compressor on them, then extract around 3 bytes to have an encoding scheme. 🎯

## 📏 Scope
I am mainly just experimenting with the compression itself. I may end up using it for something or maybe I never touch it ever again. Additionally, this is my first ever serious C project 👩‍💻, so I apologize in advance for anyone who has the displeasure of trying to figure out my undoubtedly terrible code. 🙈
