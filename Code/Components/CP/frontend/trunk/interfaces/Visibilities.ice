module askap {
    module cp {
        module cpfrontend {

            struct Visibilities {
                long bat;
            };

            interface IVisStream {
                void handle(Visibilities vis);
            };

        };
    };
};
