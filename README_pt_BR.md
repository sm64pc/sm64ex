# sm64pc
Adaptação em OpenGL de [n64decomp/sm64](https://github.com/n64decomp/sm64). 

Sinta-se livre para reportar bugs [aqui](https://github.com/sm64pc/sm64pc/issues) e contribuir, mas tenha em mente que não
aceitamos compartilhamento de conteúdo protegido com direitos autorais.

Se necessário, rode `./extract_assets.py --clean && make clean` ou `make distclean` para remover conteúdos advindos da ROM do jogo.
Este port é possível graças à [Emill](https://github.com/Emill), [n64-fast32-engine](https://github.com/Emill/n64-fast3d-engine/), e, é claro, ao
time do [n64decomp](https://github.com/n64decomp).

Em caso de contribuições, crie _pull requests_ apenas para a [branch nightly](https://github.com/sm64pc/sm64pc/tree/nightly/).
Novas funcionalidades serão adicionadas à branch master quando forem consideradas bem testadas.

*Leia isso em outras linguas: [English](README.md) [简体中文](README_zh_CN.md).*

## Recursos

 * Renderização nativa. Você agora pode jogar SM64 no PC sem precisar de emulador.
 * Proporção de tela e resolução variáveis. O jogo renderiza corretamente em basicamente qualquer tamanho de janela.
 * Suporte a entradas de controle através de `xinput`. Tal como usando DualShock 4 em distribuições Linux.
 * Controle de câmera com analógico ou mouse. (Ative com `make BETTERCAMERA=1`.)
 * Uma opção para desativar distância de renderização. (Ative com `make NODRAWINGDISTANCE=1`.)
 * Remapeamento de controles _in-game_.
 * Pule as cenas introdutórias da Peach e Lakitu com usando a opção `--skip-intro` ao executar o jogo na linha de comando.
 * Menu de cheats nas opções. (Ative com `--cheats`)
 ** Note que se algum cheat pedir pelo botão "L", o botão em questão é o "L" do N64. Certifique-se de que este está mapeado, caso necessário.

## Compilação
Para instruções de compilaçao, consulte a [wiki](https://github.com/sm64pc/sm64pc/wiki).

## Para usuários de Windows

**Certifique-se de que você tem o [MXE](mxe.cc) antes de tentar compilar em Windows. Siga o guia na Wiki.**
