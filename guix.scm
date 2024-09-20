(use-modules (guix build-system gnu))
(use-modules ((guix licenses) #:prefix license:))
(use-modules (guix packages))
(use-modules (guix gexp))

(define-public reb
  (package
    (name "reb")
    (version "0.0.1")
    (source (local-file (dirname (current-filename)) #:recursive? #t))
    (build-system gnu-build-system)
    (synopsis "Regex-based toolkit for Brainfuck")
    (description "Regex-based toolkit for Brainfuck")
    (home-page "https://github.com/aartaka/reb")
    (arguments
     '(#:tests? #f
       #:phases (modify-phases %standard-phases
                  (delete 'configure)
                  (replace 'install
                    (lambda* (#:key outputs #:allow-other-keys)
                      (let* ((out (assoc-ref outputs "out"))
                             (bin (string-append out "/bin")))
                        (mkdir-p bin)
                        (install-file "reb" bin))))
                  (delete 'validate-runpath))))
    (license license:wtfpl2)))

reb
