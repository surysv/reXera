/* Lectura de mandos USB compatibles con Gamepad API. */

(function (global) {

  "use strict";

  const DEAD_ZONE = 0.05;


  // ====================================================
  // ZONA MUERTA
  // ====================================================

  function axisValue(value) {

    if (
      !Number.isFinite(value) ||
      Math.abs(value) <= DEAD_ZONE
    ) {

      return 0;
    }

    const magnitude =
      (Math.abs(value) - DEAD_ZONE) /
      (1 - DEAD_ZONE);

    return (
      Math.sign(value) *
      Math.max(
        0,
        Math.min(1, magnitude)
      )
    );
  }


  // ====================================================
  // GAMEPAD CONTROLLER
  // ====================================================

  class GamepadController {

    constructor(onStatus) {

      this.onStatus = onStatus || function () {};

      this.index = null;

      this.prevButtons = [];

      this.lastStatus = "";


      // ----------------------------------------------
      // CONEXIÓN
      // ----------------------------------------------

      window.addEventListener(
        "gamepadconnected",
        (event) => {

          this.index = event.gamepad.index;

          this._status(event.gamepad);
        }
      );


      // ----------------------------------------------
      // DESCONEXIÓN
      // ----------------------------------------------

      window.addEventListener(
        "gamepaddisconnected",
        (event) => {

          if (
            this.index === event.gamepad.index
          ) {

            this.index = null;

            this.prevButtons = [];

            this.onStatus({
              connected: false,
              supported: false,
              text: "Mando desconectado"
            });
          }
        }
      );
    }


    // ==================================================
    // ESTADO DEL MANDO
    // ==================================================

    _status(gamepad) {

      const supported =
        !!gamepad &&
        gamepad.mapping === "standard" &&
        gamepad.axes.length >= 4 &&
        gamepad.buttons.length >= 4;

      const mappingText =
        gamepad.mapping === "standard"
          ? "mapeo estándar"
          : "mapeo no estándar";

      const text = supported

        ? `${gamepad.id} · ${mappingText}`

        : `${
            gamepad
              ? gamepad.id
              : "Mando"
          } · distribución no compatible ` +
          "(se requieren 4 ejes y 4 botones)";


      const key = `${supported}:${text}`;

      if (key !== this.lastStatus) {

        this.lastStatus = key;

        this.onStatus({
          connected: true,
          supported,
          text,
          mapping: gamepad.mapping
        });
      }

      return supported;
    }


    // ==================================================
    // LEER MANDO
    // ==================================================

    poll() {

      if (!navigator.getGamepads) {

        return this._empty(
          "Este navegador no ofrece Gamepad API"
        );
      }

      let pads;

      try {

        pads = navigator.getGamepads();

      } catch (_) {

        return this._empty(
          "No se puede consultar el mando"
        );
      }


      // ----------------------------------------------
      // LOCALIZAR CONTROLADOR
      // ----------------------------------------------

      let gamepad =
        this.index !== null
          ? pads[this.index]
          : null;

      if (
        !gamepad ||
        !gamepad.connected
      ) {

        gamepad = Array
          .from(pads)
          .find(
            (candidate) =>
              candidate && candidate.connected
          ) || null;
      }


      if (!gamepad) {

        this.index = null;

        this.prevButtons = [];

        return this._empty(
          "Conecta un mando USB"
        );
      }


      this.index = gamepad.index;


      // ----------------------------------------------
      // COMPATIBILIDAD
      // ----------------------------------------------

      const supported = this._status(gamepad);

      if (!supported) {

        this.prevButtons = [];

        return this._neutral(true, false);
      }


      // ----------------------------------------------
      // BOTONES
      // ----------------------------------------------

      const currentButtons =
        gamepad.buttons.map(
          (button) => !!button.pressed
        );

      const justPressed = (index) => (
        currentButtons[index] &&
        !this.prevButtons[index]
      );


      const buttons = {

        cross: justPressed(0),

        circle: justPressed(1),

        square: justPressed(2),

        triangle: justPressed(3)
      };

      this.prevButtons = currentButtons;


      // ----------------------------------------------
      // DEVOLVER DATOS
      // ----------------------------------------------

      return {

        connected: true,
        supported: true,

        mapping: gamepad.mapping,

        // Stick izquierdo:
        x: Math.round(
          axisValue(gamepad.axes[0]) * 100
        ),

        y: Math.round(
          -axisValue(gamepad.axes[1]) * 100
        ),

        // Stick derecho:
        servo1Axis: axisValue(
          gamepad.axes[2]
        ),

        servo2Axis: -axisValue(
          gamepad.axes[3]
        ),

        buttons
      };
    }


    // ==================================================
    // SIN MANDO
    // ==================================================

    _empty(text) {

      const key = `false:${text}`;

      if (key !== this.lastStatus) {

        this.lastStatus = key;

        this.onStatus({
          connected: false,
          supported: false,
          text
        });
      }

      return this._neutral(
        false,
        false
      );
    }


    // ==================================================
    // ESTADO NEUTRO
    // ==================================================

    _neutral(
      connected,
      supported
    ) {

      return {

        connected,
        supported,

        x: 0,
        y: 0,

        servo1Axis: 0,
        servo2Axis: 0,

        buttons: {
          cross: false,
          circle: false,
          square: false,
          triangle: false
        }
      };
    }
  }


  global.GamepadController = GamepadController;

})(window);
