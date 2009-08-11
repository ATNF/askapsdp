module askap {
    module cp {
        module frontend {

            struct Visibilities {
                long bat;
            };

            interface IVisStream {
                void handle(Visibilities vis);
            };

        };
    };
};
