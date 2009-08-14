module askap {
    module cp {
        module frontend {

            struct SimpleNumber {
                long i;
            };

            interface INumberStream {
                void handle(SimpleNumber n);
            };

        };
    };
};
